#pragma once

#include "monthmint/core/result.h"
#include "monthmint/database/budget_repo.h"
#include "monthmint/database/transaction_repo.h"
#include "monthmint/models/budget.h"
#include "monthmint/models/category.h"
#include <QVector>

namespace monthmint {

class BudgetService {
public:
    BudgetService(BudgetRepo& budgetRepo, TransactionRepo& txRepo)
        : budgetRepo_(budgetRepo), txRepo_(txRepo) {}

    [[nodiscard]] ServiceResult setOverallLimit(YearMonth month, std::optional<Money> limit);
    [[nodiscard]] ServiceResult setCategoryLimit(YearMonth month, qint64 categoryId, Money limit);
    [[nodiscard]] ServiceResult removeCategoryLimit(YearMonth month, qint64 categoryId);
    [[nodiscard]] ServiceResult copyFromPrevious(YearMonth month);

    [[nodiscard]] std::optional<Money> overallLimit(YearMonth month) const;
    [[nodiscard]] Money overallSpent(YearMonth month) const;
    [[nodiscard]] Money overallRemaining(YearMonth month) const;
    [[nodiscard]] double overallProgress(YearMonth month) const; // 0.0 .. >1.0

    struct CategoryBudgetStatus {
        qint64 categoryId = 0;
        QString categoryName;
        Money limit;
        Money spent;
        Money remaining() const { return limit - spent; }
        double progress() const {
            return limit.isPositive() ? static_cast<double>(spent.minor()) / limit.minor() : 0.0;
        }
    };
    [[nodiscard]] QVector<CategoryBudgetStatus> categoryStatus(YearMonth month,
                                                               const QVector<Category>& categories) const;

private:
    BudgetRepo& budgetRepo_;
    TransactionRepo& txRepo_;
};

} // namespace monthmint
