#pragma once

#include "monthmint/models/recurring.h"
#include <QSqlDatabase>
#include <QVector>
#include <optional>

namespace monthmint {

class RecurringRepo {
public:
    explicit RecurringRepo(QSqlDatabase db) : connectionName_(db.connectionName()) {}

    [[nodiscard]] qint64 insert(const RecurringRule& r);
    [[nodiscard]] bool update(const RecurringRule& r);
    [[nodiscard]] bool remove(qint64 id);
    [[nodiscard]] std::optional<RecurringRule> findById(qint64 id) const;
    [[nodiscard]] QVector<RecurringRule> all() const;
    [[nodiscard]] QVector<RecurringRule> activeRules() const;
    [[nodiscard]] bool markPosted(qint64 ruleId, const QDate& date);

public:
    [[nodiscard]] QSqlDatabase debugDb() const { return db(); }
private:
    [[nodiscard]] QSqlDatabase db() const { return QSqlDatabase::database(connectionName_); }
    QString connectionName_;
};

} // namespace monthmint
