#pragma once

#include <QDate>
#include <QString>
#include <optional>

namespace monthmint {

struct YearMonth {
    int year = 0;
    int month = 0; // 1..12

    [[nodiscard]] friend auto operator<=>(const YearMonth&, const YearMonth&) noexcept = default;

    [[nodiscard]] QDate firstDay() const { return QDate{year, month, 1}; }
    [[nodiscard]] QDate lastDay() const {
        const QDate first{year, month, 1};
        return QDate{year, month, first.daysInMonth()};
    }
    [[nodiscard]] QDate clamp(const QDate& d) const {
        if (d < firstDay()) return firstDay();
        if (d > lastDay()) return lastDay();
        return d;
    }
    [[nodiscard]] QString label() const;
    [[nodiscard]] QString isoKey() const;
};

[[nodiscard]] YearMonth yearMonthOf(const QDate& d);
[[nodiscard]] YearMonth previousMonth(YearMonth ym);
[[nodiscard]] YearMonth nextMonth(YearMonth ym);
[[nodiscard]] bool isValidMonth(int year, int month);
[[nodiscard]] std::optional<QDate> parseIsoDate(const QString& s);
[[nodiscard]] QString toIsoDate(const QDate& d);

} // namespace monthmint
