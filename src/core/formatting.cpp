#include "monthmint/core/formatting.h"
#include <QLocale>
#include <algorithm>
#include <limits>

namespace monthmint {

namespace {

const CurrencyInfo* findCurrency(const QString& code) {
    static const CurrencyInfo kCurrencies[] = {
        {QStringLiteral("PKR"), QStringLiteral("Rs"), 0, QStringLiteral("Pakistani Rupee")},
        {QStringLiteral("USD"), QStringLiteral("$"), 2, QStringLiteral("US Dollar")},
        {QStringLiteral("EUR"), QStringLiteral("\u20AC"), 2, QStringLiteral("Euro")},
        {QStringLiteral("GBP"), QStringLiteral("\u00A3"), 2, QStringLiteral("British Pound")},
        {QStringLiteral("JPY"), QStringLiteral("\u00A5"), 0, QStringLiteral("Japanese Yen")},
        {QStringLiteral("INR"), QStringLiteral("\u20B9"), 2, QStringLiteral("Indian Rupee")},
        {QStringLiteral("AED"), QStringLiteral("\u062F.\u0625"), 2, QStringLiteral("UAE Dirham")},
        {QStringLiteral("SAR"), QStringLiteral("\uFDFC"), 2, QStringLiteral("Saudi Riyal")},
    };
    const auto it = std::find_if(std::begin(kCurrencies), std::end(kCurrencies),
                                 [&](const CurrencyInfo& c) { return c.code == code; });
    return it != std::end(kCurrencies) ? it : nullptr;
}

/// Inserts thousands separators into a non-negative integer string.
QString groupThousands(qulonglong value, const QString& separator) {
    const QString digits = QString::number(value);
    QString out;
    int count = 0;
    for (int i = digits.size() - 1; i >= 0; --i) {
        out.prepend(digits.at(i));
        if (++count % 3 == 0 && i > 0) out.prepend(separator);
    }
    return out;
}

} // namespace

const CurrencyInfo& currencyInfo(const QString& code) {
    static const CurrencyInfo fallback{QStringLiteral("USD"), QStringLiteral("$"), 2,
                                       QStringLiteral("US Dollar")};
    if (const auto* c = findCurrency(code)) return *c;
    return fallback;
}

QStringList availableCurrencyCodes() {
    return {QStringLiteral("PKR"), QStringLiteral("USD"), QStringLiteral("EUR"),
            QStringLiteral("GBP"), QStringLiteral("JPY"), QStringLiteral("INR"),
            QStringLiteral("AED"), QStringLiteral("SAR")};
}

QString formatMoney(Money m, const QString& currencyCode, const QString& localeTag) {
    const auto& info = currencyInfo(currencyCode);
    const QLocale locale = localeTag.isEmpty() ? QLocale{QLocale::English, QLocale::UnitedStates}
                                               : QLocale{localeTag};

    QString groupSep = locale.groupSeparator();
    if (groupSep.isEmpty() || groupSep == QStringLiteral("\u00A0")) groupSep = QStringLiteral(",");
    QString decimalSep = locale.decimalPoint();
    if (decimalSep.isEmpty()) decimalSep = QStringLiteral(".");

    const bool neg = m.isNegative();
    const auto absMinor = m.abs().minor();

    QString major;
    QString minor;
    if (info.minorDigits == 0) {
        major = groupThousands(static_cast<qulonglong>(absMinor), groupSep);
    } else {
        qulonglong divisor = 1;
        for (int i = 0; i < info.minorDigits; ++i) divisor *= 10;
        const qulonglong maj = absMinor / divisor;
        const qulonglong min = absMinor % divisor;
        major = groupThousands(maj, groupSep);
        minor = QStringLiteral("%1").arg(min, info.minorDigits, 10, QLatin1Char('0'));
    }

    QString result = info.symbol + QLatin1Char(' ') + major;
    if (info.minorDigits > 0) {
        result += decimalSep + minor;
    }
    if (neg) result.prepend(QLatin1Char('-'));
    return result;
}

std::optional<Money> parseMoney(const QString& text, const QString& currencyCode) {
    const auto& info = currencyInfo(currencyCode);
    QString t = text.trimmed();
    if (t.isEmpty()) return std::nullopt;

    bool neg = false;
    if (t.startsWith(QLatin1Char('-'))) {
        neg = true;
        t = t.mid(1).trimmed();
    } else if (t.startsWith(QLatin1Char('+'))) {
        t = t.mid(1).trimmed();
    }

    // Strip currency symbol and spaces at both ends (repeat to handle "Rs 1,000").
    for (int pass = 0; pass < 4; ++pass) {
        bool changed = false;
        if (!info.symbol.isEmpty() && t.startsWith(info.symbol)) {
            t = t.mid(info.symbol.size());
            changed = true;
        }
        while (t.startsWith(QLatin1Char(' '))) {
            t = t.mid(1);
            changed = true;
        }
        if (!info.symbol.isEmpty() && t.endsWith(info.symbol)) {
            t.chop(info.symbol.size());
            changed = true;
        }
        while (t.endsWith(QLatin1Char(' '))) {
            t.chop(1);
            changed = true;
        }
        if (!changed) break;
    }

    t.remove(QLatin1Char(','));
    t.replace(QChar(0x066B), QLatin1Char('.')); // Arabic decimal separator

    int dots = 0;
    QString cleaned;
    for (const QChar c : t) {
        if (c.isDigit()) cleaned += c;
        else if (c == QLatin1Char('.')) {
            if (++dots > 1) return std::nullopt;
            cleaned += c;
        } else if (!c.isSpace() && !c.isNull()) {
            return std::nullopt;
        }
    }
    if (cleaned.isEmpty()) return std::nullopt;

    constexpr auto kMax = std::numeric_limits<Money::Value>::max();

    if (info.minorDigits == 0) {
        QString whole = cleaned;
        const auto dotIdx = cleaned.indexOf(QLatin1Char('.'));
        if (dotIdx >= 0) {
            const auto frac = cleaned.mid(dotIdx + 1);
            for (const QChar c : frac) {
                if (c != QLatin1Char('0')) return std::nullopt;
            }
            whole = cleaned.left(dotIdx);
        }
        bool ok = false;
        const auto v = whole.toLongLong(&ok);
        if (!ok) return std::nullopt;
        if (neg) {
            if (v > 0 && static_cast<qulonglong>(v) > static_cast<qulonglong>(kMax) + 1ULL)
                return std::nullopt;
            return Money{-v};
        }
        if (v < 0 || static_cast<qulonglong>(v) > static_cast<qulonglong>(kMax))
            return std::nullopt;
        return Money{v};
    }

    QString whole = cleaned;
    QString frac;
    const auto dotIdx = cleaned.indexOf(QLatin1Char('.'));
    if (dotIdx >= 0) {
        whole = cleaned.left(dotIdx);
        frac = cleaned.mid(dotIdx + 1);
    }
    while (frac.size() < info.minorDigits) frac += QLatin1Char('0');
    if (frac.size() > info.minorDigits) frac.truncate(info.minorDigits);
    if (whole.isEmpty()) whole = QStringLiteral("0");

    bool ok1 = false, ok2 = false;
    const auto w = whole.toULongLong(&ok1);
    const auto f = frac.toULongLong(&ok2);
    if (!ok1 || !ok2) return std::nullopt;

    qulonglong divisor = 1;
    for (int i = 0; i < info.minorDigits; ++i) divisor *= 10;

    // Overflow check: w * divisor + f must fit in an int64 (plus one for negation).
    const qulonglong limit = static_cast<qulonglong>(kMax) + (neg ? 1ULL : 0ULL);
    if (w != 0 && divisor > limit / w) return std::nullopt;
    const qulonglong total = w * divisor + f;
    if (total > limit) return std::nullopt;

    const auto minor = static_cast<Money::Value>(total);
    return Money{neg ? -minor : minor};
}

} // namespace monthmint
