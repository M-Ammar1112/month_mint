#include "monthmint/ui/dashboard_page.h"
#include "monthmint/core/formatting.h"
#include "monthmint/ui/app_context.h"
#include "monthmint/ui/month_selector.h"
#include "monthmint/ui/progress_widget.h"
#include "monthmint/ui/theme.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>

namespace monthmint::ui {

namespace {

QFrame* makeCard(const QString& title, QLabel*& valueOut, QWidget* parent) {
    auto* card = new QFrame(parent);
    card->setObjectName(QStringLiteral("Card"));
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(6);

    auto* titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("CardTitle"));
    layout->addWidget(titleLabel);

    auto* value = new QLabel(QStringLiteral("—"), card);
    value->setObjectName(QStringLiteral("BigAmount"));
    layout->addWidget(value);
    valueOut = value;
    return card;
}

} // namespace

DashboardPage::DashboardPage(AppContext& ctx, QWidget* parent) : QWidget(parent), ctx_(ctx) {
    month_ = yearMonthOf(QDate::currentDate());
    buildUi();
    refresh();
}

void DashboardPage::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    root->addWidget(scroll);

    auto* content = new QWidget(scroll);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(18);

    // Header
    auto* header = new QHBoxLayout;
    auto* titles = new QVBoxLayout;
    auto* title = new QLabel(QStringLiteral("Dashboard"), content);
    title->setObjectName(QStringLiteral("Title"));
    auto* subtitle = new QLabel(QStringLiteral("Your money. One simple monthly view."), content);
    subtitle->setObjectName(QStringLiteral("Subtitle"));
    titles->addWidget(title);
    titles->addWidget(subtitle);
    header->addLayout(titles);
    header->addStretch();
    monthSelector_ = new MonthSelector(content);
    monthSelector_->setMonth(month_);
    header->addWidget(monthSelector_);
    layout->addLayout(header);

    connect(monthSelector_, &MonthSelector::monthChanged, this, [this](YearMonth ym) {
        setMonth(ym);
    });

    // Summary cards
    auto* cardsGrid = new QGridLayout;
    cardsGrid->setSpacing(14);
    cardsGrid->addWidget(makeCard(QStringLiteral("Income this month"), incomeValue_, content), 0, 0);
    cardsGrid->addWidget(makeCard(QStringLiteral("Expenses this month"), expenseValue_, content), 0, 1);
    cardsGrid->addWidget(makeCard(QStringLiteral("Remaining"), remainingValue_, content), 0, 2);
    cardsGrid->addWidget(makeCard(QStringLiteral("Saved this month"), savingsValue_, content), 0, 3);
    layout->addLayout(cardsGrid);

    // Budget + savings progress
    auto* progressRow = new QHBoxLayout;
    progressRow->setSpacing(14);

    auto* budgetCard = new QFrame(content);
    budgetCard->setObjectName(QStringLiteral("Card"));
    auto* budgetLayout = new QVBoxLayout(budgetCard);
    budgetLayout->setContentsMargins(16, 14, 16, 14);
    budgetLayout->setSpacing(10);
    auto* budgetTitle = new QLabel(QStringLiteral("Monthly budget"), budgetCard);
    budgetTitle->setObjectName(QStringLiteral("CardTitle"));
    budgetProgress_ = new ProgressWidget(budgetCard);
    budgetDetail_ = new QLabel(QStringLiteral("No budget set"), budgetCard);
    budgetDetail_->setObjectName(QStringLiteral("Subtitle"));
    budgetLayout->addWidget(budgetTitle);
    budgetLayout->addWidget(budgetProgress_);
    budgetLayout->addWidget(budgetDetail_);
    progressRow->addWidget(budgetCard, 1);

    auto* savingsCard = new QFrame(content);
    savingsCard->setObjectName(QStringLiteral("Card"));
    auto* savingsLayout = new QVBoxLayout(savingsCard);
    savingsLayout->setContentsMargins(16, 14, 16, 14);
    savingsLayout->setSpacing(10);
    auto* savingsTitle = new QLabel(QStringLiteral("Savings target"), savingsCard);
    savingsTitle->setObjectName(QStringLiteral("CardTitle"));
    savingsProgress_ = new ProgressWidget(savingsCard);
    savingsLayout->addWidget(savingsTitle);
    savingsLayout->addWidget(savingsProgress_);
    progressRow->addWidget(savingsCard, 1);

    layout->addLayout(progressRow);

    // Category breakdown
    auto* catCard = new QFrame(content);
    catCard->setObjectName(QStringLiteral("Card"));
    auto* catLayout = new QVBoxLayout(catCard);
    catLayout->setContentsMargins(16, 14, 16, 16);
    catLayout->setSpacing(10);
    auto* catTitle = new QLabel(QStringLiteral("Spending by category"), catCard);
    catTitle->setObjectName(QStringLiteral("CardTitle"));
    catLayout->addWidget(catTitle);
    categoryLayout_ = new QVBoxLayout;
    categoryLayout_->setSpacing(8);
    catLayout->addLayout(categoryLayout_);
    layout->addWidget(catCard);

    // Recent transactions
    auto* recentCard = new QFrame(content);
    recentCard->setObjectName(QStringLiteral("Card"));
    auto* recentLayout = new QVBoxLayout(recentCard);
    recentLayout->setContentsMargins(16, 14, 16, 16);
    recentLayout->setSpacing(10);
    auto* recentTitle = new QLabel(QStringLiteral("Recent transactions"), recentCard);
    recentTitle->setObjectName(QStringLiteral("CardTitle"));
    recentLayout->addWidget(recentTitle);

    recentTable_ = new QTableWidget(recentCard);
    recentTable_->setColumnCount(4);
    recentTable_->setHorizontalHeaderLabels(
        {QStringLiteral("Date"), QStringLiteral("Category"), QStringLiteral("Description"),
         QStringLiteral("Amount")});
    recentTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    recentTable_->verticalHeader()->setVisible(false);
    recentTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recentTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentTable_->setAlternatingRowColors(true);
    recentTable_->setShowGrid(false);
    recentTable_->setFixedHeight(210);
    recentLayout->addWidget(recentTable_);

    recentEmpty_ = new QLabel(QStringLiteral("No transactions yet. Add your first expense to get started."), recentCard);
    recentEmpty_->setObjectName(QStringLiteral("Subtitle"));
    recentEmpty_->setAlignment(Qt::AlignCenter);
    recentLayout->addWidget(recentEmpty_);

    layout->addWidget(recentCard);
    layout->addStretch();

    scroll->setWidget(content);
}

void DashboardPage::setMonth(YearMonth ym) {
    month_ = ym;
    if (monthSelector_->month() != ym) monthSelector_->setMonth(ym);
    refresh();
}

void DashboardPage::refresh() {
    updateSummaryCards();
    updateCategoryBreakdown();
    updateBudgetProgress();
    updateRecent();
}

void DashboardPage::updateSummaryCards() {
    const auto summary = ctx_.summaryService.monthlySummary(month_);
    const auto& cur = ctx_.settings.currencyCode;

    incomeValue_->setText(formatMoney(summary.income, cur));
    incomeValue_->setStyleSheet(QStringLiteral("color: %1;").arg(positiveColor()));

    expenseValue_->setText(formatMoney(summary.expenses, cur));
    expenseValue_->setStyleSheet(QStringLiteral("color: %1;").arg(dangerColor()));

    const auto remaining = summary.netCashFlow();
    remainingValue_->setText(formatMoney(remaining, cur));
    remainingValue_->setStyleSheet(
        QStringLiteral("color: %1;").arg(remaining.isNegative() ? dangerColor() : QStringLiteral("#17202B")));

    savingsValue_->setText(formatMoney(summary.savingsAllocated, cur));

    if (summary.savingsTarget.isPositive()) {
        savingsProgress_->setProgress(summary.savingsProgress());
        savingsProgress_->setLabelText(QStringLiteral("%1 / %2")
                                           .arg(formatMoney(summary.savingsAllocated, cur),
                                                formatMoney(summary.savingsTarget, cur)));
    } else {
        savingsProgress_->setProgress(0.0);
        savingsProgress_->setLabelText(QStringLiteral("No target set"));
    }
}

void DashboardPage::updateBudgetProgress() {
    const auto limit = ctx_.budgetService.overallLimit(month_);
    const auto spent = ctx_.budgetService.overallSpent(month_);
    const auto& cur = ctx_.settings.currencyCode;

    if (!limit || !limit->isPositive()) {
        budgetProgress_->setProgress(0.0);
        budgetProgress_->setLabelText(QStringLiteral("—"));
        budgetDetail_->setText(QStringLiteral("No budget set for %1").arg(month_.label()));
        return;
    }
    const double p = ctx_.budgetService.overallProgress(month_);
    budgetProgress_->setProgress(p);
    budgetProgress_->setLabelText(QStringLiteral("%1%").arg(qRound(p * 100)));
    const auto remaining = *limit - spent;
    budgetDetail_->setText(QStringLiteral("%1 spent of %2 · %3 left")
                               .arg(formatMoney(spent, cur), formatMoney(*limit, cur),
                                    formatMoney(remaining, cur)));
}

void DashboardPage::updateCategoryBreakdown() {
    while (QLayoutItem* item = categoryLayout_->takeAt(0)) {
        if (auto* w = item->widget()) w->deleteLater();
        delete item;
    }

    const auto categories = ctx_.categoryRepo.all();
    const auto spends = ctx_.summaryService.spendingByCategory(month_, categories);

    if (spends.isEmpty()) {
        auto* empty = new QLabel(QStringLiteral("No expenses recorded this month."), this);
        empty->setObjectName(QStringLiteral("Subtitle"));
        categoryLayout_->addWidget(empty);
        return;
    }

    for (const auto& s : spends) {
        auto* row = new QWidget(this);
        auto* rl = new QVBoxLayout(row);
        rl->setContentsMargins(0, 0, 0, 0);
        rl->setSpacing(4);
        auto* header = new QHBoxLayout;
        auto* name = new QLabel(s.name, row);
        auto* amount = new QLabel(formatMoney(s.spent, ctx_.settings.currencyCode), row);
        amount->setAlignment(Qt::AlignRight);
        header->addWidget(name);
        header->addStretch();
        header->addWidget(amount);
        rl->addLayout(header);

        auto* bar = new ProgressWidget(row);
        bar->setProgress(s.share, 2.0, 2.0);
        bar->setLabelText(QStringLiteral("%1%").arg(qRound(s.share * 100)));
        bar->setShowPercent(false);
        bar->setStyleSheet(QStringLiteral("QProgressBar::chunk { background: %1; border-radius: 6px; }")
                               .arg(positiveColor()));
        rl->addWidget(bar);
        categoryLayout_->addWidget(row);
    }
}

void DashboardPage::updateRecent() {
    const auto recent = ctx_.txRepo.recent(5);
    recentTable_->setRowCount(recent.size());
    for (int i = 0; i < recent.size(); ++i) {
        const auto& t = recent[i];
        recentTable_->setItem(i, 0, new QTableWidgetItem(t.date.toString(QStringLiteral("dd MMM"))));
        recentTable_->setItem(i, 1, new QTableWidgetItem(t.categoryName));
        recentTable_->setItem(i, 2, new QTableWidgetItem(t.description));
        auto* amountItem = new QTableWidgetItem(formatMoney(t.amount, ctx_.settings.currencyCode));
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setForeground(t.type == TransactionType::Income ? QColor(positiveColor())
                                                                     : QColor(dangerColor()));
        recentTable_->setItem(i, 3, amountItem);
    }
    recentTable_->resizeColumnsToContents();
    recentTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    const bool hasRows = !recent.isEmpty();
    recentTable_->setVisible(hasRows);
    recentEmpty_->setVisible(!hasRows);
}

} // namespace monthmint::ui
