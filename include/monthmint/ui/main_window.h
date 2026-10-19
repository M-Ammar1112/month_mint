#pragma once

#include "monthmint/core/date_utils.h"
#include "monthmint/ui/app_context.h"
#include <QMainWindow>

class QStackedWidget;
class QPushButton;
class QFrame;

namespace monthmint::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(std::unique_ptr<AppContext> ctx, QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void buildUi();
    void buildSidebar();
    void switchPage(int index);
    void refreshAll();
    void changeMonth(YearMonth ym);
    void addExpense();
    void addIncome();
    void openSettings();
    void checkRecurringOnStartup();

    std::unique_ptr<AppContext> ctx_;
    YearMonth month_;
    QStackedWidget* stack_ = nullptr;
    QFrame* sidebar_ = nullptr;
    class DashboardPage* dashboard_ = nullptr;
    class TransactionsPage* transactions_ = nullptr;
    class BudgetsPage* budgets_ = nullptr;
    class SavingsPage* savings_ = nullptr;
    QPushButton* navDashboard_ = nullptr;
    QPushButton* navTransactions_ = nullptr;
    QPushButton* navBudgets_ = nullptr;
    QPushButton* navSavings_ = nullptr;
};

} // namespace monthmint::ui
