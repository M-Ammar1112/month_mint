#include "monthmint/ui/first_run_dialog.h"
#include "monthmint/core/formatting.h"
#include "monthmint/ui/money_edit.h"
#include "monthmint/ui/theme.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace monthmint::ui {

FirstRunDialog::FirstRunDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("Welcome to MonthMint"));
    setMinimumWidth(440);
    buildUi();
}

void FirstRunDialog::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("Welcome to MonthMint"), this);
    title->setObjectName(QStringLiteral("Title"));
    auto* subtitle = new QLabel(
        QStringLiteral("Your money. One simple monthly view.\n"
                       "A few quick choices — everything is optional and changeable later."),
        this);
    subtitle->setObjectName(QStringLiteral("Subtitle"));
    subtitle->setWordWrap(true);
    root->addWidget(title);
    root->addWidget(subtitle);

    auto* form = new QFormLayout;
    form->setSpacing(12);

    currencyCombo_ = new QComboBox(this);
    for (const auto& code : availableCurrencyCodes()) {
        const auto& info = currencyInfo(code);
        currencyCombo_->addItem(QStringLiteral("%1 (%2)").arg(code, info.symbol), code);
    }
    currencyCombo_->setCurrentIndex(currencyCombo_->findData(QStringLiteral("USD")));

    themeCombo_ = new QComboBox(this);
    themeCombo_->addItem(QStringLiteral("Light"), QStringLiteral("light"));
    themeCombo_->addItem(QStringLiteral("Dark"), QStringLiteral("dark"));

    budgetEdit_ = new MoneyEdit(this);
    budgetEdit_->setPlaceholderText(QStringLiteral("Optional"));

    savingsEdit_ = new MoneyEdit(this);
    savingsEdit_->setPlaceholderText(QStringLiteral("Optional"));

    form->addRow(QStringLiteral("Currency"), currencyCombo_);
    form->addRow(QStringLiteral("Theme"), themeCombo_);
    form->addRow(QStringLiteral("Monthly budget"), budgetEdit_);
    form->addRow(QStringLiteral("Savings target"), savingsEdit_);
    root->addLayout(form);

    connect(currencyCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) {
        budgetEdit_->setCurrencyCode(currencyCombo_->currentData().toString());
        savingsEdit_->setCurrencyCode(currencyCombo_->currentData().toString());
    });

    auto* buttons = new QDialogButtonBox(this);
    auto* skip = buttons->addButton(QStringLiteral("Skip"), QDialogButtonBox::RejectRole);
    skip->setObjectName(QStringLiteral("SecondaryButton"));
    auto* start = buttons->addButton(QStringLiteral("Get started"), QDialogButtonBox::AcceptRole);
    start->setObjectName(QStringLiteral("PrimaryButton"));
    connect(skip, &QPushButton::clicked, this, [this] {
        choices_.currencyCode = currencyCombo_->currentData().toString();
        choices_.theme = themeCombo_->currentData().toString();
        accept();
    });
    connect(start, &QPushButton::clicked, this, &FirstRunDialog::finish);
    root->addWidget(buttons);
}

void FirstRunDialog::finish() {
    choices_.currencyCode = currencyCombo_->currentData().toString();
    choices_.theme = themeCombo_->currentData().toString();

    if (const auto b = budgetEdit_->value()) {
        if (b->isPositive()) choices_.monthlyBudgetMinor = b->minor();
    }
    if (const auto s = savingsEdit_->value()) {
        if (s->isPositive()) choices_.savingsTargetMinor = s->minor();
    }
    accept();
}

} // namespace monthmint::ui
