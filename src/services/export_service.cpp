#include "monthmint/services/export_service.h"
#include "monthmint/core/csv.h"
#include "monthmint/core/date_utils.h"

#include <QFile>
#include <QTextStream>

namespace monthmint {

bool ExportService::exportToCsv(const TransactionFilter& filter, const QString& filePath,
                                QString* errorOut) const {
    const auto rows = txRepo_.query(filter);
    QFile file{filePath};
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorOut) *errorOut = file.errorString();
        return false;
    }

    QTextStream out{&file};
    out.setEncoding(QStringConverter::Utf8);
    out << toCsvRow({QStringLiteral("Date"), QStringLiteral("Type"), QStringLiteral("Category"),
                     QStringLiteral("Description"), QStringLiteral("AmountMinor")})
        << QStringLiteral("\n");

    for (const auto& t : rows) {
        out << toCsvRow({toIsoDate(t.date), transactionTypeKey(t.type), t.categoryName,
                         t.description, QString::number(t.amount.minor())})
            << QStringLiteral("\n");
    }
    return true;
}

} // namespace monthmint
