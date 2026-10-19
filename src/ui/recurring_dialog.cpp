#include "monthmint/ui/recurring_dialog.h"
#include "monthmint/core/formatting.h"
#include "monthmint/ui/app_context.h"
#include "monthmint/ui/money_edit.h"
#include "monthmint/ui/theme.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

namespace monthmint::ui {

namespace {

class RuleEditor : public QDialog {
public:
    RuleEditor(AppContext& ctx, std::optional<RecurringRule> existing, QWidget* parent)
        : QDialog(parent), ctx_(ctx), existing_(std::move(existing)) {
        setWindowTitle(existing_ ? QStringLiteral("Edit Recurring Rule")
                                 : QStringLiteral("New Recurring Rule"));
        auto* layout = new QVBoxLayout(this);
        auto* form = new QFormLayout;

        typeCombo_ = new QComboBox(this);
        typeCombo_->addItem(QStringLiteral("Expense"), static_cast<int>(TransactionType::Expense));
        typeCombo_->addItem(QStringLiteral("Income"), static_cast<int>(TransactionType::Income));

        amountEdit_ = new MoneyEdit(this);
        amountEdit_->setCurrencyCode(ctx.settings.currencyCode);

        categoryCombo_ = new QComboBox(this);

        descEdit_ = new QLineEdit(this);
        daySpin_ = new QSpinBox(this);
        daySpin_->setRange(1, 31);
        daySpin_->setValue(1);

        form->addRow(QStringLiteral("Type"), typeCombo_);
        form->addRow(QStringLiteral("Amount"), amountEdit_);
        form->addRow(QStringLiteral("Category"), categoryCombo_);
        form->addRow(QStringLiteral("Description"), descEdit_);
        form->addRow(QStringLiteral("Day of month"), daySpin_);
        layout->addLayout(form);

        auto* buttons = new QDialogButtonBox(this);
        buttons->addButton(QStringLiteral("Cancel"), QDialogButtonBox::RejectRole);
        auto* saveBtn = buttons->addButton(QStringLiteral("Save"), QDialogButtonBox::AcceptRole);
        saveBtn->setObjectName(QStringLiteral("PrimaryButton"));
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        layout->addWidget(buttons);

        connect(typeCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this,
                [this](int) { reloadCategories(); });
        reloadCategories();

        if (existing_) {
            typeCombo_->setCurrentIndex(existing_->type == TransactionType::Income ? 1 : 0);
            reloadCategories();
            amountEdit_->setValue(existing_->amount);
            categoryCombo_->setCurrentIndex(categoryCombo_->findData(QVariant::fromValue(existing_->categoryId)));
            descEdit_->setText(existing_->description);
            daySpin_->setValue(existing_->dayOfMonth);
        }
    }

    [[nodiscard]] std::optional<RecurringRule> result() const { return result_; }

protected:
    void accept() override {
        const auto amount = amountEdit_->value();
        if (!amount || !amount->isPositive()) {
            QMessageBox::warning(this, QStringLiteral("Invalid amount"),
                                 QStringLiteral("Enter an amount greater than zero."));
            return;
        }
        const auto catId = categoryCombo_->currentData().toLongLong();
        if (catId <= 0) {
            QMessageBox::warning(this, QStringLiteral("Category required"),
                                 QStringLiteral("Select a category."));
            return;
        }
        RecurringRule r = existing_.value_or(RecurringRule{});
        r.type = static_cast<TransactionType>(typeCombo_->currentData().toInt());
        r.amount = *amount;
        r.categoryId = catId;
        r.description = descEdit_->text().trimmed();
        r.dayOfMonth = daySpin_->value();
        r.active = existing_ ? existing_->active : true;
        result_ = r;
        QDialog::accept();
    }

private:
    void reloadCategories() {
        categoryCombo_->clear();
        const auto wanted = static_cast<TransactionType>(typeCombo_->currentData().toInt());
        for (const auto& c : ctx_.categoryRepo.all()) {
            const bool matches = wanted == TransactionType::Income
                                     ? c.kind == CategoryKind::Income || c.kind == CategoryKind::Both
                                     : c.kind == CategoryKind::Expense || c.kind == CategoryKind::Both;
            if (matches) categoryCombo_->addItem(c.name, QVariant::fromValue(c.id));
        }
    }

    AppContext& ctx_;
    std::optional<RecurringRule> existing_;
    std::optional<RecurringRule> result_;
    QComboBox* typeCombo_ = nullptr;
    MoneyEdit* amountEdit_ = nullptr;
    QComboBox* categoryCombo_ = nullptr;
    QLineEdit* descEdit_ = nullptr;
    QSpinBox* daySpin_ = nullptr;
};

} // namespace

RecurringDialog::RecurringDialog(AppContext& ctx, QWidget* parent) : QDialog(parent), ctx_(ctx) {
    setWindowTitle(QStringLiteral("Recurring transactions"));
    resize(720, 520);
    buildUi();
    reload();
}

void RecurringDialog::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    auto* tabs = new QTabWidget(this);

    // Rules tab
    auto* rulesTab = new QWidget(tabs);
    auto* rl = new QVBoxLayout(rulesTab);
    rulesTable_ = new QTableWidget(rulesTab);
    rulesTable_->setColumnCount(5);
    rulesTable_->setHorizontalHeaderLabels({QStringLiteral("Type"), QStringLiteral("Category"),
                                            QStringLiteral("Description"), QStringLiteral("Day"),
                                            QStringLiteral("Amount")});
    rulesTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    rulesTable_->verticalHeader()->setVisible(false);
    rulesTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    rulesTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    rulesTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rl->addWidget(rulesTable_);

    auto* ruleButtons = new QHBoxLayout;
    auto* addBtn = new QPushButton(QStringLiteral("Add rule"), rulesTab);
    addBtn->setObjectName(QStringLiteral("PrimaryButton"));
    auto* editBtn = new QPushButton(QStringLiteral("Edit"), rulesTab);
    editBtn->setObjectName(QStringLiteral("SecondaryButton"));
    auto* removeBtn = new QPushButton(QStringLiteral("Delete"), rulesTab);
    removeBtn->setObjectName(QStringLiteral("DangerButton"));
    ruleButtons->addStretch();
    ruleButtons->addWidget(addBtn);
    ruleButtons->addWidget(editBtn);
    ruleButtons->addWidget(removeBtn);
    rl->addLayout(ruleButtons);
    tabs->addTab(rulesTab, QStringLiteral("Rules"));

    // Due tab
    auto* dueTab = new QWidget(tabs);
    auto* dl = new QVBoxLayout(dueTab);
    auto* dueHint = new QLabel(
        QStringLiteral("These entries are due this month and have not been posted yet."), dueTab);
    dueHint->setObjectName(QStringLiteral("Subtitle"));
    dl->addWidget(dueHint);
    dueTable_ = new QTableWidget(dueTab);
    dueTable_->setColumnCount(4);
    dueTable_->setHorizontalHeaderLabels({QStringLiteral("Due"), QStringLiteral("Type"),
                                          QStringLiteral("Category"), QStringLiteral("Amount")});
    dueTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    dueTable_->verticalHeader()->setVisible(false);
    dueTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    dueTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    dueTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    dl->addWidget(dueTable_);
    auto* dueButtons = new QHBoxLayout;
    auto* postBtn = new QPushButton(QStringLiteral("Post selected"), dueTab);
    postBtn->setObjectName(QStringLiteral("PrimaryButton"));
    dueButtons->addStretch();
    dueButtons->addWidget(postBtn);
    dl->addLayout(dueButtons);
    tabs->addTab(dueTab, QStringLiteral("Due now"));

    root->addWidget(tabs);

    auto* buttons = new QDialogButtonBox(this);
    auto* close = buttons->addButton(QStringLiteral("Close"), QDialogButtonBox::RejectRole);
    close->setObjectName(QStringLiteral("SecondaryButton"));
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);

    connect(addBtn, &QPushButton::clicked, this, &RecurringDialog::addRule);
    connect(editBtn, &QPushButton::clicked, this, [this] { editRule(rulesTable_->currentRow()); });
    connect(removeBtn, &QPushButton::clicked, this, [this] { removeRule(rulesTable_->currentRow()); });
    connect(rulesTable_, &QTableWidget::doubleClicked, this,
            [this](const QModelIndex&) { editRule(rulesTable_->currentRow()); });
    connect(postBtn, &QPushButton::clicked, this, [this] { postSelected(dueTable_->currentRow()); });
}

void RecurringDialog::reload() {
    const auto rules = ctx_.recurringService.allRules();
    rulesTable_->setRowCount(rules.size());
    for (int i = 0; i < rules.size(); ++i) {
        const auto& r = rules[i];
        auto* typeItem = new QTableWidgetItem(transactionTypeLabel(r.type));
        typeItem->setData(Qt::UserRole, QVariant::fromValue(r.id));
        rulesTable_->setItem(i, 0, typeItem);
        rulesTable_->setItem(i, 1, new QTableWidgetItem(r.categoryName));
        rulesTable_->setItem(i, 2, new QTableWidgetItem(r.description));
        rulesTable_->setItem(i, 3, new QTableWidgetItem(QString::number(r.dayOfMonth)));
        auto* amt = new QTableWidgetItem(formatMoney(r.amount, ctx_.settings.currencyCode));
        amt->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rulesTable_->setItem(i, 4, amt);
    }
    rulesTable_->resizeColumnsToContents();
    rulesTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    const auto due = ctx_.recurringService.duePreviews(QDate::currentDate());
    dueTable_->setRowCount(due.size());
    for (int i = 0; i < due.size(); ++i) {
        const auto& p = due[i];
        auto* dueItem = new QTableWidgetItem(p.dueDate.toString(QStringLiteral("dd MMM yyyy")));
        dueItem->setData(Qt::UserRole, i);
        dueTable_->setItem(i, 0, dueItem);
        dueTable_->setItem(i, 1, new QTableWidgetItem(transactionTypeLabel(p.rule.type)));
        dueTable_->setItem(i, 2, new QTableWidgetItem(p.rule.categoryName));
        auto* amt = new QTableWidgetItem(formatMoney(p.rule.amount, ctx_.settings.currencyCode));
        amt->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        dueTable_->setItem(i, 3, amt);
    }
    dueTable_->resizeColumnsToContents();
    dueTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
}

void RecurringDialog::addRule() {
    RuleEditor editor(ctx_, std::nullopt, this);
    if (editor.exec() != QDialog::Accepted) return;
    if (auto rule = editor.result()) {
        const auto r = ctx_.recurringService.addRule(*rule);
        if (!r.ok) {
            QMessageBox::warning(this, QStringLiteral("Could not save"), r.error);
            return;
        }
        reload();
        emit dataChanged();
    }
}

void RecurringDialog::editRule(int row) {
    if (row < 0) return;
    const auto id = rulesTable_->item(row, 0)->data(Qt::UserRole).toLongLong();
    const auto rule = ctx_.recurringRepo.findById(id);
    if (!rule) return;
    RuleEditor editor(ctx_, *rule, this);
    if (editor.exec() != QDialog::Accepted) return;
    if (auto updated = editor.result()) {
        const auto r = ctx_.recurringService.updateRule(*updated);
        if (!r.ok) {
            QMessageBox::warning(this, QStringLiteral("Could not update"), r.error);
            return;
        }
        reload();
        emit dataChanged();
    }
}

void RecurringDialog::removeRule(int row) {
    if (row < 0) return;
    const auto id = rulesTable_->item(row, 0)->data(Qt::UserRole).toLongLong();
    const auto answer = QMessageBox::question(this, QStringLiteral("Delete rule"),
                                              QStringLiteral("Delete this recurring rule?"),
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;
    ctx_.recurringService.removeRule(id);
    reload();
    emit dataChanged();
}

void RecurringDialog::postSelected(int row) {
    if (row < 0) {
        QMessageBox::information(this, QStringLiteral("Select an entry"),
                                 QStringLiteral("Select a due entry to post."));
        return;
    }
    const int previewIndex = dueTable_->item(row, 0)->data(Qt::UserRole).toInt();
    const auto due = ctx_.recurringService.duePreviews(QDate::currentDate());
    if (previewIndex < 0 || previewIndex >= due.size()) return;

    const auto r = ctx_.recurringService.post(due[previewIndex]);
    if (!r.ok) {
        QMessageBox::warning(this, QStringLiteral("Could not post"), r.error);
        return;
    }
    reload();
    emit dataChanged();
}

} // namespace monthmint::ui
