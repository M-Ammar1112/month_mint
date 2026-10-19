#include "monthmint/ui/savings_page.h"
#include "monthmint/core/formatting.h"
#include "monthmint/ui/app_context.h"
#include "monthmint/ui/money_edit.h"
#include "monthmint/ui/month_selector.h"
#include "monthmint/ui/progress_widget.h"
#include "monthmint/ui/theme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace monthmint::ui {

SavingsPage::SavingsPage(AppContext& ctx, QWidget* parent) : QWidget(parent), ctx_(ctx) {
    month_ = yearMonthOf(QDate::currentDate());
    buildUi();
    refresh();
}

void SavingsPage::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 24);
    root->setSpacing(16);

    auto* header = new QHBoxLayout;
    auto* title = new QLabel(QStringLiteral("Savings"), this);
    title->setObjectName(QStringLiteral("Title"));
    header->addWidget(title);
    header->addStretch();
    monthSelector_ = new MonthSelector(this);
    monthSelector_->setMonth(month_);
    header->addWidget(monthSelector_);
    root->addLayout(header);

    connect(monthSelector_, &MonthSelector::monthChanged, this, [this](YearMonth ym) {
        setMonth(ym);
    });

    auto* targetCard = new QFrame(this);
    targetCard->setObjectName(QStringLiteral("Card"));
    auto* tl = new QVBoxLayout(targetCard);
    tl->setContentsMargins(16, 14, 16, 16);
    tl->setSpacing(10);
    auto* targetTitle = new QLabel(QStringLiteral("Monthly savings target"), targetCard);
    targetTitle->setObjectName(QStringLiteral("CardTitle"));
    tl->addWidget(targetTitle);

    auto* targetRow = new QHBoxLayout;
    targetEdit_ = new MoneyEdit(targetCard);
    targetEdit_->setCurrencyCode(ctx_.settings.currencyCode);
    targetEdit_->setPlaceholderText(QStringLiteral("0.00"));
    auto* saveTargetBtn = new QPushButton(QStringLiteral("Save target"), targetCard);
    saveTargetBtn->setObjectName(QStringLiteral("PrimaryButton"));
    targetRow->addWidget(targetEdit_, 1);
    targetRow->addWidget(saveTargetBtn);
    tl->addLayout(targetRow);

    progress_ = new ProgressWidget(targetCard);
    allocatedLabel_ = new QLabel(targetCard);
    allocatedLabel_->setObjectName(QStringLiteral("Subtitle"));
    tl->addWidget(progress_);
    tl->addWidget(allocatedLabel_);

    auto* hint = new QLabel(
        QStringLiteral("Savings allocations are tracked separately and never counted as expenses."),
        targetCard);
    hint->setObjectName(QStringLiteral("Subtitle"));
    hint->setWordWrap(true);
    tl->addWidget(hint);
    root->addWidget(targetCard);

    auto* allocCard = new QFrame(this);
    allocCard->setObjectName(QStringLiteral("Card"));
    auto* al = new QVBoxLayout(allocCard);
    al->setContentsMargins(16, 14, 16, 16);
    al->setSpacing(10);
    auto* allocTitle = new QLabel(QStringLiteral("Add to savings"), allocCard);
    allocTitle->setObjectName(QStringLiteral("CardTitle"));
    al->addWidget(allocTitle);

    auto* allocRow = new QHBoxLayout;
    allocEdit_ = new MoneyEdit(allocCard);
    allocEdit_->setCurrencyCode(ctx_.settings.currencyCode);
    allocEdit_->setPlaceholderText(QStringLiteral("Amount"));
    allocNote_ = new QLineEdit(allocCard);
    allocNote_->setPlaceholderText(QStringLiteral("Note (optional)"));
    auto* allocBtn = new QPushButton(QStringLiteral("Allocate"), allocCard);
    allocBtn->setObjectName(QStringLiteral("PrimaryButton"));
    allocRow->addWidget(allocEdit_);
    allocRow->addWidget(allocNote_, 1);
    allocRow->addWidget(allocBtn);
    al->addLayout(allocRow);
    root->addWidget(allocCard);
    root->addStretch();

    connect(saveTargetBtn, &QPushButton::clicked, this, [this] {
        const auto v = targetEdit_->value();
        if (!v) {
            QMessageBox::warning(this, QStringLiteral("Invalid target"),
                                 QStringLiteral("Enter a valid amount."));
            return;
        }
        const auto r = ctx_.savingsService.setTarget(month_, *v);
        if (!r.ok) {
            QMessageBox::warning(this, QStringLiteral("Could not save"), r.error);
            return;
        }
        updateProgress();
        emit dataChanged();
    });
    connect(allocBtn, &QPushButton::clicked, this, &SavingsPage::onAllocate);
}

void SavingsPage::setMonth(YearMonth ym) {
    month_ = ym;
    if (monthSelector_->month() != ym) monthSelector_->setMonth(ym);
    refresh();
}

void SavingsPage::refresh() { updateProgress(); }

void SavingsPage::updateProgress() {
    const auto target = ctx_.savingsService.target(month_);
    const auto allocated = ctx_.savingsService.allocated(month_);
    const auto& cur = ctx_.settings.currencyCode;

    if (target) targetEdit_->setValue(*target);
    else targetEdit_->clearValue();

    allocatedLabel_->setText(QStringLiteral("Allocated: %1").arg(formatMoney(allocated, cur)));

    if (target && target->isPositive()) {
        const double p = static_cast<double>(allocated.minor()) / target->minor();
        progress_->setProgress(p);
        progress_->setLabelText(QStringLiteral("%1 / %2").arg(formatMoney(allocated, cur),
                                                             formatMoney(*target, cur)));
    } else {
        progress_->setProgress(0.0);
        progress_->setLabelText(QStringLiteral("No target set"));
    }
}

void SavingsPage::onAllocate() {
    const auto v = allocEdit_->value();
    if (!v || !v->isPositive()) {
        QMessageBox::warning(this, QStringLiteral("Invalid amount"),
                             QStringLiteral("Enter an amount greater than zero."));
        return;
    }
    const auto r = ctx_.savingsService.allocate(month_, *v, allocNote_->text().trimmed());
    if (!r.ok) {
        QMessageBox::warning(this, QStringLiteral("Could not allocate"), r.error);
        return;
    }
    allocEdit_->clearValue();
    allocNote_->clear();
    updateProgress();
    emit dataChanged();
}

} // namespace monthmint::ui
