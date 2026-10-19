#pragma once

#include <QString>

namespace monthmint {

struct AppSettings {
    QString currencyCode = QStringLiteral("USD");
    QString localeTag;                     // empty = system default
    QString theme = QStringLiteral("light"); // "light" | "dark"
    bool firstRunCompleted = false;
};

} // namespace monthmint
