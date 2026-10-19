#pragma once

#include "monthmint/models/budget.h"
#include <QSqlDatabase>
#include <QVector>
#include <optional>

namespace monthmint {

class BudgetRepo {
public:
    explicit BudgetRepo(QSqlDatabase db) : connectionName_(db.connectionName()) {}

    [[nodiscard]] std::optional<Money> overallLimit(YearMonth month) const;
    [[nodiscard]] bool setOverallLimit(YearMonth month, std::optional<Money> limit);
    [[nodiscard]] QVector<CategoryBudget> categoryLimits(YearMonth month) const;
    [[nodiscard]] bool setCategoryLimit(YearMonth month, qint64 categoryId, Money limit);
    [[nodiscard]] bool removeCategoryLimit(YearMonth month, qint64 categoryId);
    [[nodiscard]] bool copyFromPreviousMonth(YearMonth month);

public:
    [[nodiscard]] QSqlDatabase debugDb() const { return db(); }
private:
    [[nodiscard]] QSqlDatabase db() const { return QSqlDatabase::database(connectionName_); }
    QString connectionName_;
};

} // namespace monthmint
