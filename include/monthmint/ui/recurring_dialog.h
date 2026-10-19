#pragma once

#include <QDialog>

class QTableWidget;

namespace monthmint::ui {

class AppContext;

/// Manage monthly recurring rules and review/post due entries.
class RecurringDialog : public QDialog {
    Q_OBJECT
public:
    explicit RecurringDialog(AppContext& ctx, QWidget* parent = nullptr);

signals:
    void dataChanged();

private:
    void buildUi();
    void addRule();
    void editRule(int row);
    void removeRule(int row);
    void postSelected(int row);
    void reload();

    AppContext& ctx_;
    QTableWidget* rulesTable_ = nullptr;
    QTableWidget* dueTable_ = nullptr;
};

} // namespace monthmint::ui
