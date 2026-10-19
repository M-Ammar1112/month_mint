#pragma once

#include "monthmint/models/savings.h"
#include <QSqlDatabase>
#include <optional>

namespace monthmint {

class SavingsRepo {
public:
    explicit SavingsRepo(QSqlDatabase db) : connectionName_(db.connectionName()) {}

    [[nodiscard]] std::optional<Money> target(YearMonth month) const;
    [[nodiscard]] bool setTarget(YearMonth month, Money target);
    [[nodiscard]] Money allocated(YearMonth month) const;
    [[nodiscard]] qint64 addAllocation(YearMonth month, Money amount, const QString& note);
    [[nodiscard]] bool removeAllocation(qint64 id);

public:
    [[nodiscard]] QSqlDatabase debugDb() const { return db(); }
private:
    [[nodiscard]] QSqlDatabase db() const { return QSqlDatabase::database(connectionName_); }
    QString connectionName_;
};

} // namespace monthmint
