#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace monthmint {

[[nodiscard]] QString csvEscape(const QString& field);
[[nodiscard]] QString toCsvRow(const QStringList& fields);
[[nodiscard]] QStringList parseCsvRow(const QString& line);

} // namespace monthmint
