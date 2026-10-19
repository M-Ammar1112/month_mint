#pragma once

#include "monthmint/core/date_utils.h"
#include "monthmint/models/transaction.h"
#include <QSqlDatabase>
#include <QVector>
#include <optional>

namespace monthmint {

struct TransactionFilter {
    std::optional<YearMonth> month;
    std::optional<qint64> categoryId;
    std::optional<TransactionType> type;
    QString search;
    int limit = 0;
};

class TransactionRepo {
public:
    explicit TransactionRepo(QSqlDatabase db) : connectionName_(db.connectionName()) {}

    [[nodiscard]] qint64 insert(const Transaction& t);
    [[nodiscard]] bool update(const Transaction& t);
    [[nodiscard]] bool remove(qint64 id);
    [[nodiscard]] std::optional<Transaction> findById(qint64 id) const;
    [[nodiscard]] QVector<Transaction> query(const TransactionFilter& filter) const;
    [[nodiscard]] Money sumByType(TransactionType type, YearMonth month) const;
    [[nodiscard]] Money sumByCategory(qint64 categoryId, YearMonth month) const;
    [[nodiscard]] QVector<Transaction> recent(int limit) const;

private:
    [[nodiscard]] QSqlDatabase db() const { return QSqlDatabase::database(connectionName_); }
public:
    [[nodiscard]] QSqlDatabase debugDb() const { return db(); }
    QString connectionName_;
};

} // namespace monthmint
