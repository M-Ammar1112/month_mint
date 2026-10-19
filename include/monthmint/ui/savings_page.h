#pragma once

#include "monthmint/core/date_utils.h"
#include <QWidget>

class QLabel;
class QLineEdit;

namespace monthmint::ui {

class AppContext;
class MonthSelector;
class MoneyEdit;
class ProgressWidget;

class SavingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SavingsPage(AppContext& ctx, QWidget* parent = nullptr);

    void setMonth(YearMonth ym);
    void refresh();

signals:
    void dataChanged();

private:
    void buildUi();
    void updateProgress();
    void onAllocate();

    AppContext& ctx_;
    YearMonth month_;
    MonthSelector* monthSelector_ = nullptr;
    MoneyEdit* targetEdit_ = nullptr;
    MoneyEdit* allocEdit_ = nullptr;
    QLineEdit* allocNote_ = nullptr;
    QLabel* allocatedLabel_ = nullptr;
    ProgressWidget* progress_ = nullptr;
};

} // namespace monthmint::ui
