#pragma once

#include <QDialog>
#include <optional>

class QComboBox;

namespace monthmint::ui {

class MoneyEdit;

/// First-run wizard: currency selection plus optional budget and savings target.
/// All optional steps can be skipped.
class FirstRunDialog : public QDialog {
    Q_OBJECT
public:
    struct Choices {
        QString currencyCode = QStringLiteral("USD");
        QString theme = QStringLiteral("light");
        std::optional<qint64> monthlyBudgetMinor;
        std::optional<qint64> savingsTargetMinor;
    };

    explicit FirstRunDialog(QWidget* parent = nullptr);

    [[nodiscard]] Choices choices() const { return choices_; }

private:
    void buildUi();
    void finish();

    Choices choices_;
    QComboBox* currencyCombo_ = nullptr;
    QComboBox* themeCombo_ = nullptr;
    MoneyEdit* budgetEdit_ = nullptr;
    MoneyEdit* savingsEdit_ = nullptr;
};

} // namespace monthmint::ui
