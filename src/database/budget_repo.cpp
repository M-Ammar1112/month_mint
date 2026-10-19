#include "monthmint/database/budget_repo.h"
#include "monthmint/core/date_utils.h"

#include <QSqlQuery>
#include <QVariant>

namespace monthmint {

std::optional<Money> BudgetRepo::overallLimit(YearMonth month) const {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "SELECT overall_limit_minor FROM monthly_budgets WHERE month_key = ?"));
    q.addBindValue(month.isoKey());
    if (q.exec() && q.next()) {
        if (q.value(0).isNull()) return std::nullopt;
        return Money{q.value(0).toLongLong()};
    }
    return std::nullopt;
}

bool BudgetRepo::setOverallLimit(YearMonth month, std::optional<Money> limit) {
    const auto key = month.isoKey();
    if (!limit) {
        QSqlQuery q{db()};
        q.prepare(QStringLiteral("DELETE FROM monthly_budgets WHERE month_key = ?"));
        q.addBindValue(key);
        return q.exec();
    }
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "INSERT INTO monthly_budgets (month_key, overall_limit_minor) VALUES (?, ?) "
        "ON CONFLICT(month_key) DO UPDATE SET overall_limit_minor = excluded.overall_limit_minor"));
    q.addBindValue(key);
    q.addBindValue(static_cast<qlonglong>(limit->minor()));
    return q.exec();
}

QVector<CategoryBudget> BudgetRepo::categoryLimits(YearMonth month) const {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "SELECT category_id, limit_minor FROM category_budgets WHERE month_key = ?"));
    q.addBindValue(month.isoKey());
    QVector<CategoryBudget> result;
    if (!q.exec()) return result;
    while (q.next()) {
        result.push_back(CategoryBudget{month, q.value(0).toLongLong(), Money{q.value(1).toLongLong()}});
    }
    return result;
}

bool BudgetRepo::setCategoryLimit(YearMonth month, qint64 categoryId, Money limit) {
    if (limit.isNegative()) return false;
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "INSERT INTO category_budgets (month_key, category_id, limit_minor) VALUES (?, ?, ?) "
        "ON CONFLICT(month_key, category_id) DO UPDATE SET limit_minor = excluded.limit_minor"));
    q.addBindValue(month.isoKey());
    q.addBindValue(categoryId);
    q.addBindValue(static_cast<qlonglong>(limit.minor()));
    return q.exec();
}

bool BudgetRepo::removeCategoryLimit(YearMonth month, qint64 categoryId) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "DELETE FROM category_budgets WHERE month_key = ? AND category_id = ?"));
    q.addBindValue(month.isoKey());
    q.addBindValue(categoryId);
    return q.exec() && q.numRowsAffected() > 0;
}

bool BudgetRepo::copyFromPreviousMonth(YearMonth month) {
    const auto prev = previousMonth(month);

    // Copy overall limit
    if (const auto prevOverall = overallLimit(prev)) {
        if (!setOverallLimit(month, prevOverall)) return false;
    }

    // Copy category limits
    for (const auto& cb : categoryLimits(prev)) {
        if (!setCategoryLimit(month, cb.categoryId, cb.limit)) return false;
    }
    return true;
}

} // namespace monthmint
