#include "monthmint/ui/transactions_page.h"
#include "monthmint/core/formatting.h"
#include "monthmint/ui/add_transaction_dialog.h"
#include "monthmint/ui/app_context.h"
#include "monthmint/ui/month_selector.h"
#include "monthmint/ui/theme.h"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace monthmint::ui {

TransactionsPage::TransactionsPage(AppContext& ctx, QWidget* parent) : QWidget(parent), ctx_(ctx) {
    buildUi();
    reloadTable();
}

void TransactionsPage::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 24);
    root->setSpacing(16);

    auto* header = new QHBoxLayout;
    auto* title = new QLabel(QStringLiteral("Transactions"), this);
    title->setObjectName(QStringLiteral("Title"));
    header->addWidget(title);
    header->addStretch();
    monthSelector_ = new MonthSelector(this);
    monthSelector_->setMonth(yearMonthOf(QDate::currentDate()));
    header->addWidget(monthSelector_);
    root->addLayout(header);

    connect(monthSelector_, &MonthSelector::monthChanged, this, [this](YearMonth ym) {
        setMonth(ym);
    });

    auto* toolbar = new QHBoxLayout;
    toolbar->setSpacing(10);

    typeFilter_ = new QComboBox(this);
    typeFilter_->addItem(QStringLiteral("All types"), QVariant{});
    typeFilter_->addItem(QStringLiteral("Expenses"), static_cast<int>(TransactionType::Expense));
    typeFilter_->addItem(QStringLiteral("Income"), static_cast<int>(TransactionType::Income));
    typeFilter_->addItem(QStringLiteral("Savings"), static_cast<int>(TransactionType::SavingsAllocation));

    categoryFilter_ = new QComboBox(this);
    categoryFilter_->addItem(QStringLiteral("All categories"), 0);

    searchEdit_ = new QLineEdit(this);
    searchEdit_->setPlaceholderText(QStringLiteral("Search description…"));
    searchEdit_->setClearButtonEnabled(true);

    auto* addBtn = new QPushButton(QStringLiteral("+ Add"), this);
    addBtn->setObjectName(QStringLiteral("PrimaryButton"));

    toolbar->addWidget(typeFilter_);
    toolbar->addWidget(categoryFilter_);
    toolbar->addWidget(searchEdit_, 1);
    toolbar->addWidget(addBtn);
    root->addLayout(toolbar);

    connect(addBtn, &QPushButton::clicked, this, &TransactionsPage::onAdd);
    connect(typeFilter_, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int) { applyFilter(); });
    connect(categoryFilter_, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int) { applyFilter(); });
    connect(searchEdit_, &QLineEdit::textChanged, this, [this](const QString&) { applyFilter(); });

    table_ = new QTableWidget(this);
    table_->setColumnCount(5);
    table_->setHorizontalHeaderLabels({QStringLiteral("Date"), QStringLiteral("Type"),
                                       QStringLiteral("Category"), QStringLiteral("Description"),
                                       QStringLiteral("Amount")});
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    table_->verticalHeader()->setVisible(false);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setAlternatingRowColors(true);
    table_->setShowGrid(false);
    root->addWidget(table_, 1);

    emptyLabel_ = new QLabel(QStringLiteral("No transactions match your filters."), this);
    emptyLabel_->setObjectName(QStringLiteral("Subtitle"));
    emptyLabel_->setAlignment(Qt::AlignCenter);
    root->addWidget(emptyLabel_);

    auto* actions = new QHBoxLayout;
    auto* editBtn = new QPushButton(QStringLiteral("Edit"), this);
    editBtn->setObjectName(QStringLiteral("SecondaryButton"));
    auto* deleteBtn = new QPushButton(QStringLiteral("Delete"), this);
    deleteBtn->setObjectName(QStringLiteral("DangerButton"));
    actions->addStretch();
    actions->addWidget(editBtn);
    actions->addWidget(deleteBtn);
    root->addLayout(actions);

    connect(editBtn, &QPushButton::clicked, this, &TransactionsPage::onEdit);
    connect(deleteBtn, &QPushButton::clicked, this, &TransactionsPage::onDelete);
    connect(table_, &QTableWidget::doubleClicked, this, [this](const QModelIndex&) { onEdit(); });

    reloadCategories();
}

void TransactionsPage::reloadCategories() {
    const qint64 current = categoryFilter_->currentData().toLongLong();
    categoryFilter_->blockSignals(true);
    categoryFilter_->clear();
    categoryFilter_->addItem(QStringLiteral("All categories"), 0);
    for (const auto& c : ctx_.categoryRepo.all()) {
        categoryFilter_->addItem(c.name, QVariant::fromValue(c.id));
    }
    const int idx = categoryFilter_->findData(QVariant::fromValue(current));
    if (idx >= 0) categoryFilter_->setCurrentIndex(idx);
    categoryFilter_->blockSignals(false);
}

void TransactionsPage::setMonth(YearMonth ym) {
    if (monthSelector_->month() != ym) monthSelector_->setMonth(ym);
    reloadTable();
}

void TransactionsPage::refresh() {
    reloadCategories();
    reloadTable();
}

void TransactionsPage::applyFilter() { reloadTable(); }

void TransactionsPage::reloadTable() {
    TransactionFilter filter;
    filter.month = monthSelector_->month();
    if (typeFilter_->currentData().isValid())
        filter.type = static_cast<TransactionType>(typeFilter_->currentData().toInt());
    const auto catId = categoryFilter_->currentData().toLongLong();
    if (catId > 0) filter.categoryId = catId;
    filter.search = searchEdit_->text().trimmed();

    const auto rows = ctx_.transactionService.search(filter);
    table_->setRowCount(rows.size());
    for (int i = 0; i < rows.size(); ++i) {
        const auto& t = rows[i];
        table_->setItem(i, 0, new QTableWidgetItem(t.date.toString(QStringLiteral("yyyy-MM-dd"))));
        table_->setItem(i, 1, new QTableWidgetItem(transactionTypeLabel(t.type)));
        table_->setItem(i, 2, new QTableWidgetItem(t.categoryName));
        table_->setItem(i, 3, new QTableWidgetItem(t.description));

        auto* amount = new QTableWidgetItem(formatMoney(t.amount, ctx_.settings.currencyCode));
        amount->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QString color = dangerColor();
        if (t.type == TransactionType::Income) color = positiveColor();
        else if (t.type == TransactionType::SavingsAllocation) color = QStringLiteral("#2563EB");
        amount->setForeground(QColor(color));
        amount->setData(Qt::UserRole, QVariant::fromValue(t.id));
        table_->setItem(i, 4, amount);
    }

    if (auto* item = table_->item(0, 0)) {
        table_->selectRow(0);
        Q_UNUSED(item);
    }
    table_->resizeColumnsToContents();
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    const bool empty = rows.isEmpty();
    table_->setVisible(!empty);
    emptyLabel_->setVisible(empty);
}

void TransactionsPage::onAdd() {
    AddTransactionDialog dlg(AddTransactionDialog::Mode::Expense, ctx_, this);
    if (dlg.exec() == QDialog::Accepted) {
        reloadTable();
        emit dataChanged();
    }
}

void TransactionsPage::onEdit() {
    const int row = table_->currentRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("Select a transaction"),
                                 QStringLiteral("Select a transaction to edit."));
        return;
    }
    const auto id = table_->item(row, 4)->data(Qt::UserRole).toLongLong();
    const auto tx = ctx_.txRepo.findById(id);
    if (!tx) return;
    if (AddTransactionDialog::editTransaction(ctx_, *tx, this)) {
        reloadTable();
        emit dataChanged();
    }
}

void TransactionsPage::onDelete() {
    const int row = table_->currentRow();
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("Select a transaction"),
                                 QStringLiteral("Select a transaction to delete."));
        return;
    }
    const auto id = table_->item(row, 4)->data(Qt::UserRole).toLongLong();
    const auto answer = QMessageBox::question(
        this, QStringLiteral("Delete transaction"),
        QStringLiteral("Delete the selected transaction? This cannot be undone."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    const auto r = ctx_.transactionService.removeTransaction(id);
    if (!r.ok) {
        QMessageBox::warning(this, QStringLiteral("Could not delete"), r.error);
        return;
    }
    reloadTable();
    emit dataChanged();
}

} // namespace monthmint::ui
