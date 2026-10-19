#pragma once

#include "monthmint/core/date_utils.h"
#include "monthmint/models/transaction.h"
#include <QWidget>

class QComboBox;
class QLineEdit;
class QTableWidget;
class QLabel;

namespace monthmint::ui {

class AppContext;
class MonthSelector;

class TransactionsPage : public QWidget {
    Q_OBJECT
public:
    explicit TransactionsPage(AppContext& ctx, QWidget* parent = nullptr);

    void refresh();
    void setMonth(YearMonth ym);

signals:
    void dataChanged();

private:
    void buildUi();
    void reloadTable();
    void reloadCategories();
    void applyFilter();
    void onAdd();
    void onEdit();
    void onDelete();

    AppContext& ctx_;
    MonthSelector* monthSelector_ = nullptr;
    QComboBox* typeFilter_ = nullptr;
    QComboBox* categoryFilter_ = nullptr;
    QLineEdit* searchEdit_ = nullptr;
    QTableWidget* table_ = nullptr;
    QLabel* emptyLabel_ = nullptr;
};

} // namespace monthmint::ui
