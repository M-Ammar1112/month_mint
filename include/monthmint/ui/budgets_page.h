#pragma once

#include "monthmint/core/date_utils.h"
#include <QWidget>

class QLabel;
class QTableWidget;

namespace monthmint::ui {

class AppContext;
class MonthSelector;
class MoneyEdit;
class ProgressWidget;

class BudgetsPage : public QWidget {
    Q_OBJECT
public:
    explicit BudgetsPage(AppContext& ctx, QWidget* parent = nullptr);

    void setMonth(YearMonth ym);
    void refresh();

signals:
    void dataChanged();

private:
    void buildUi();
    void updateOverall();
    void updateCategories();

    AppContext& ctx_;
    YearMonth month_;
    MonthSelector* monthSelector_ = nullptr;
    MoneyEdit* overallEdit_ = nullptr;
    QLabel* overallStatus_ = nullptr;
    ProgressWidget* overallProgress_ = nullptr;
    QTableWidget* categoryTable_ = nullptr;
};

} // namespace monthmint::ui
