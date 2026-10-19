#pragma once

#include "monthmint/core/date_utils.h"
#include <QWidget>

class QLabel;
class QPushButton;

namespace monthmint::ui {

/// Compact "< January 2026 >" month navigation control.
class MonthSelector : public QWidget {
    Q_OBJECT
public:
    explicit MonthSelector(QWidget* parent = nullptr);

    void setMonth(YearMonth ym);
    [[nodiscard]] YearMonth month() const { return current_; }

signals:
    void monthChanged(YearMonth ym);

private:
    void refresh();

    YearMonth current_;
    QPushButton* prev_ = nullptr;
    QPushButton* next_ = nullptr;
    QLabel* label_ = nullptr;
};

} // namespace monthmint::ui
