#pragma once

#include "monthmint/core/date_utils.h"
#include <QWidget>

class QLabel;
class QVBoxLayout;
class QTableWidget;

namespace monthmint::ui {

class AppContext;
class MonthSelector;
class ProgressWidget;

class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(AppContext& ctx, QWidget* parent = nullptr);

    void setMonth(YearMonth ym);
    void refresh();

signals:
    void navigateToTransactions();

private:
    void buildUi();
    void updateSummaryCards();
    void updateCategoryBreakdown();
    void updateBudgetProgress();
    void updateRecent();

    AppContext& ctx_;
    YearMonth month_;
    MonthSelector* monthSelector_ = nullptr;
    QLabel* incomeValue_ = nullptr;
    QLabel* expenseValue_ = nullptr;
    QLabel* remainingValue_ = nullptr;
    QLabel* savingsValue_ = nullptr;
    ProgressWidget* savingsProgress_ = nullptr;
    ProgressWidget* budgetProgress_ = nullptr;
    QLabel* budgetDetail_ = nullptr;
    QVBoxLayout* categoryLayout_ = nullptr;
    QTableWidget* recentTable_ = nullptr;
    QLabel* recentEmpty_ = nullptr;
};

} // namespace monthmint::ui
