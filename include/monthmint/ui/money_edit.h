#pragma once

#include "monthmint/core/money.h"
#include <QLineEdit>
#include <optional>

namespace monthmint::ui {

/// Line edit specialized for monetary input in major currency units.
class MoneyEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit MoneyEdit(QWidget* parent = nullptr);

    void setCurrencyCode(const QString& code);
    [[nodiscard]] QString currencyCode() const;

    /// Parses current text; nullopt when empty or invalid.
    [[nodiscard]] std::optional<Money> value() const;

    void setValue(Money minorUnits);
    void clearValue();

    [[nodiscard]] bool hasValidValue() const;

signals:
    void valueChanged();

private:
    QString currencyCode_ = QStringLiteral("USD");
};

} // namespace monthmint::ui
