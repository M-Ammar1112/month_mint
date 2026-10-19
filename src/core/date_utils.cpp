#include "monthmint/core/date_utils.h"

namespace monthmint {

QString YearMonth::label() const {
    static const char* names[] = {
        "", "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    if (month < 1 || month > 12) return {};
    return QStringLiteral("%1 %2").arg(QString::fromLatin1(names[month])).arg(year);
}

QString YearMonth::isoKey() const {
    return QStringLiteral("%1-%2").arg(year, 4, 10, QLatin1Char('0'))
                                   .arg(month, 2, 10, QLatin1Char('0'));
}

YearMonth yearMonthOf(const QDate& d) {
    return YearMonth{d.year(), d.month()};
}

YearMonth previousMonth(YearMonth ym) {
    if (ym.month == 1) return YearMonth{ym.year - 1, 12};
    return YearMonth{ym.year, ym.month - 1};
}

YearMonth nextMonth(YearMonth ym) {
    if (ym.month == 12) return YearMonth{ym.year + 1, 1};
    return YearMonth{ym.year, ym.month + 1};
}

bool isValidMonth(int year, int month) {
    return month >= 1 && month <= 12 && year >= 1900 && year <= 2200;
}

std::optional<QDate> parseIsoDate(const QString& s) {
    const QDate d = QDate::fromString(s, Qt::ISODate);
    return d.isValid() ? std::optional{d} : std::nullopt;
}

QString toIsoDate(const QDate& d) {
    return d.toString(Qt::ISODate);
}

} // namespace monthmint
