#pragma once

#include <QString>

namespace monthmint::ui {

/// Applies the light or dark theme stylesheet to the application.
void applyTheme(const QString& themeName);
[[nodiscard]] QString themeStylesheet(const QString& themeName);

// Semantic colors used across pages (resolved after applyTheme).
[[nodiscard]] QString positiveColor();
[[nodiscard]] QString warningColor();
[[nodiscard]] QString dangerColor();

} // namespace monthmint::ui
