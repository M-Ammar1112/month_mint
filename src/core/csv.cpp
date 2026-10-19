#include "monthmint/core/csv.h"

namespace monthmint {

QString csvEscape(const QString& field) {
    const bool needsQuoting = field.contains(QLatin1Char(',')) ||
                              field.contains(QLatin1Char('"')) ||
                              field.contains(QLatin1Char('\n')) ||
                              field.contains(QLatin1Char('\r'));
    if (!needsQuoting) return field;
    QString escaped = field;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

QString toCsvRow(const QStringList& fields) {
    QStringList escaped;
    escaped.reserve(fields.size());
    for (const auto& f : fields) escaped << csvEscape(f);
    return escaped.join(QLatin1Char(','));
}

QStringList parseCsvRow(const QString& line) {
    QStringList fields;
    QString field;
    bool inQuotes = false;
    for (int i = 0; i < line.size(); ++i) {
        const QChar c = line[i];
        if (inQuotes) {
            if (c == QLatin1Char('"')) {
                if (i + 1 < line.size() && line[i + 1] == QLatin1Char('"')) {
                    field += QLatin1Char('"');
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                field += c;
            }
        } else {
            if (c == QLatin1Char('"')) {
                inQuotes = true;
            } else if (c == QLatin1Char(',')) {
                fields << field;
                field.clear();
            } else {
                field += c;
            }
        }
    }
    fields << field;
    return fields;
}

} // namespace monthmint
