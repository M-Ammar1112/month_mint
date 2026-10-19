#pragma once

#include "monthmint/core/money.h"
#include <QString>

namespace monthmint {

struct CurrencyInfo {
    QString code;       // "PKR", "USD", ...
    QString symbol;     // "Rs", "$", ...
    int minorDigits;    // digits after decimal point in major unit
    QString name;
};

[[nodiscard]] const CurrencyInfo& currencyInfo(const QString& code);
[[nodiscard]] QStringList availableCurrencyCodes();
[[nodiscard]] QString formatMoney(Money m, const QString& currencyCode, const QString& localeTag = {});
[[nodiscard]] std::optional<Money> parseMoney(const QString& text, const QString& currencyCode);

} // namespace monthmint
