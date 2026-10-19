#pragma once

#include "monthmint/database/transaction_repo.h"
#include <QString>

namespace monthmint {

class ExportService {
public:
    explicit ExportService(TransactionRepo& txRepo) : txRepo_(txRepo) {}

    [[nodiscard]] bool exportToCsv(const TransactionFilter& filter, const QString& filePath,
                                   QString* errorOut = nullptr) const;

private:
    TransactionRepo& txRepo_;
};

} // namespace monthmint
