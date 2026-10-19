#include "monthmint/ui/add_transaction_dialog.h"
#include "monthmint/ui/app_context.h"
#include "monthmint/ui/money_edit.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace monthmint::ui {

AddTransactionDialog::AddTransactionDialog(Mode mode, AppContext& ctx, QWidget* parent)
    : QDialog(parent), mode_(mode), ctx_(ctx) {
    setWindowTitle(mode == Mode::Expense ? QStringLiteral("Add Expense")
                                         : QStringLiteral("Add Income"));
    setMinimumWidth(380);
    buildUi();
    loadCategories();

    const auto type = mode == Mode::Expense ? TransactionType::Expense : TransactionType::Income;
    selectedCategoryId_ = ctx_.categoryRepo.all().isEmpty()
                              ? 0
                              : [&] {
                                    for (const auto& c : ctx_.categoryRepo.all()) {
                                        if (c.kind == CategoryKind::Expense && type == TransactionType::Expense)
                                            return c.id;
                                        if (c.kind == CategoryKind::Income && type == TransactionType::Income)
                                            return c.id;
                                    }
                                    return qint64{0};
                                }();
    dateEdit_->setDate(QDate::currentDate());
    amountEdit_->setFocus();
}

void AddTransactionDialog::buildUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(14);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* title = new QLabel(mode_ == Mode::Expense ? QStringLiteral("Record an expense")
                                                    : QStringLiteral("Record income"), this);
    title->setObjectName(QStringLiteral("Title"));
    layout->addWidget(title);

    auto* form = new QFormLayout;
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignLeft);

    amountEdit_ = new MoneyEdit(this);
    amountEdit_->setCurrencyCode(ctx_.settings.currencyCode);
    amountEdit_->setPlaceholderText(QStringLiteral("0.00"));

    categoryCombo_ = new QComboBox(this);

    dateEdit_ = new QDateEdit(this);
    dateEdit_->setCalendarPopup(true);
    dateEdit_->setDisplayFormat(QStringLiteral("dd MMM yyyy"));

    descEdit_ = new QLineEdit(this);
    descEdit_->setPlaceholderText(QStringLiteral("Optional note"));

    form->addRow(QStringLiteral("Amount"), amountEdit_);
    form->addRow(QStringLiteral("Category"), categoryCombo_);
    form->addRow(QStringLiteral("Date"), dateEdit_);
    form->addRow(QStringLiteral("Description"), descEdit_);
    layout->addLayout(form);
    layout->addStretch();

    auto* buttons = new QDialogButtonBox(this);
    auto* cancel = buttons->addButton(QStringLiteral("Cancel"), QDialogButtonBox::RejectRole);
    saveBtn_ = buttons->addButton(QStringLiteral("Save"), QDialogButtonBox::AcceptRole);
    saveBtn_->setObjectName(QStringLiteral("PrimaryButton"));
    cancel->setObjectName(QStringLiteral("SecondaryButton"));
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(saveBtn_, &QPushButton::clicked, this, &AddTransactionDialog::accept);
    connect(amountEdit_, &MoneyEdit::valueChanged, this, [this] {
        saveBtn_->setEnabled(amountEdit_->hasValidValue());
    });
    saveBtn_->setEnabled(false);
    layout->addWidget(buttons);
}

void AddTransactionDialog::loadCategories() {
    categoryCombo_->clear();
    const auto type = mode_ == Mode::Expense ? CategoryKind::Expense : CategoryKind::Income;
    for (const auto& c : ctx_.categoryRepo.all()) {
        const bool matches = (c.kind == type) || (c.kind == CategoryKind::Both);
        if (matches) categoryCombo_->addItem(c.name, QVariant::fromValue(c.id));
    }
    if (categoryCombo_->count() > 0) selectedCategoryId_ = categoryCombo_->currentData().toLongLong();
    connect(categoryCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) {
        selectedCategoryId_ = categoryCombo_->currentData().toLongLong();
    });
}

void AddTransactionDialog::accept() {
    const auto amount = amountEdit_->value();
    if (!amount || !amount->isPositive()) {
        QMessageBox::warning(this, QStringLiteral("Invalid amount"),
                             QStringLiteral("Enter an amount greater than zero."));
        return;
    }
    if (selectedCategoryId_ <= 0) {
        QMessageBox::warning(this, QStringLiteral("Category required"),
                             QStringLiteral("Select a category."));
        return;
    }

    Transaction t;
    t.type = mode_ == Mode::Expense ? TransactionType::Expense : TransactionType::Income;
    t.amount = *amount;
    t.date = dateEdit_->date();
    t.categoryId = selectedCategoryId_;
    t.description = descEdit_->text().trimmed();

    ServiceResult r;
    if (mode_ == Mode::Expense)
        r = ctx_.transactionService.addExpense(t.amount, t.date, t.categoryId, t.description);
    else
        r = ctx_.transactionService.addIncome(t.amount, t.date, t.categoryId, t.description);

    if (!r.ok) {
        QMessageBox::warning(this, QStringLiteral("Could not save"), r.error);
        return;
    }
    result_ = t;
    result_.id = r.id;
    QDialog::accept();
}

std::optional<Transaction> AddTransactionDialog::editTransaction(AppContext& ctx, Transaction t,
                                                                 QWidget* parent) {
    const auto mode = t.type == TransactionType::Income ? Mode::Income : Mode::Expense;
    AddTransactionDialog dlg(mode, ctx, parent);
    dlg.setWindowTitle(QStringLiteral("Edit Transaction"));
    dlg.editing_ = t;
    dlg.categoryCombo_->setCurrentIndex(
        dlg.categoryCombo_->findData(QVariant::fromValue(t.categoryId)));
    dlg.amountEdit_->setValue(t.amount);
    dlg.dateEdit_->setDate(t.date);
    dlg.descEdit_->setText(t.description);
    dlg.saveBtn_->setEnabled(true);
    dlg.saveBtn_->setText(QStringLiteral("Update"));

    connect(dlg.saveBtn_, &QPushButton::clicked, &dlg, [&dlg, &ctx, t] {
        const auto amount = dlg.amountEdit_->value();
        if (!amount || !amount->isPositive()) {
            QMessageBox::warning(&dlg, QStringLiteral("Invalid amount"),
                                 QStringLiteral("Enter an amount greater than zero."));
            return;
        }
        Transaction updated = t;
        updated.amount = *amount;
        updated.date = dlg.dateEdit_->date();
        updated.categoryId = dlg.selectedCategoryId_;
        updated.description = dlg.descEdit_->text().trimmed();
        const auto r = ctx.transactionService.updateTransaction(updated);
        if (!r.ok) {
            QMessageBox::warning(&dlg, QStringLiteral("Could not update"), r.error);
            return;
        }
        dlg.result_ = updated;
        dlg.done(QDialog::Accepted);
    }, Qt::QueuedConnection);

    if (dlg.exec() == QDialog::Accepted) return dlg.result_;
    return std::nullopt;
}

} // namespace monthmint::ui
