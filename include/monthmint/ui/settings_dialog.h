#pragma once

#include <QDialog>

class QComboBox;

namespace monthmint::ui {

class AppContext;
class MoneyEdit;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(AppContext& ctx, QWidget* parent = nullptr);

signals:
    void settingsChanged();

private:
    void buildUi();
    void save();
    void exportCsv();
    void backup();
    void restore();

    AppContext& ctx_;
    QComboBox* currencyCombo_ = nullptr;
    QComboBox* localeCombo_ = nullptr;
    QComboBox* themeCombo_ = nullptr;
};

} // namespace monthmint::ui
