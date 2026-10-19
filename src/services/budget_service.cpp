#include "monthmint/services/budget_service.h"
#include "monthmint/models/category.h"
#include <algorithm>

namespace monthmint {

ServiceResult BudgetService::setOverallLimit(YearMonth month, std::optional<Money> limit) {
    if (limit && limit->isNegative())
        return ServiceResult::failure(QStringLiteral("Budget cannot be negative."));
    if (!budgetRepo_.setOverallLimit(month, limit))
        return ServiceResult::failure(QStringLiteral("Failed to save overall budget."));
    return ServiceResult::success();
}

ServiceResult BudgetService::setCategoryLimit(YearMonth month, qint64 categoryId, Money limit) {
    if (categoryId <= 0)
        return ServiceResult::failure(QStringLiteral("Category is required."));
    if (limit.isNegative())
        return ServiceResult::failure(QStringLiteral("Category limit cannot be negative."));
    if (!budgetRepo_.setCategoryLimit(month, categoryId, limit))
        return ServiceResult::failure(QStringLiteral("Failed to save category budget."));
    return ServiceResult::success();
}

ServiceResult BudgetService::removeCategoryLimit(YearMonth month, qint64 categoryId) {
    if (!budgetRepo_.removeCategoryLimit(month, categoryId))
        return ServiceResult::failure(QStringLiteral("Failed to remove category budget."));
    return ServiceResult::success();
}

ServiceResult BudgetService::copyFromPrevious(YearMonth month) {
    if (!budgetRepo_.copyFromPreviousMonth(month))
        return ServiceResult::failure(QStringLiteral("Failed to copy budgets."));
    return ServiceResult::success();
}

std::optional<Money> BudgetService::overallLimit(YearMonth month) const {
    return budgetRepo_.overallLimit(month);
}

Money BudgetService::overallSpent(YearMonth month) const {
    return txRepo_.sumByType(TransactionType::Expense, month);
}

Money BudgetService::overallRemaining(YearMonth month) const {
    const auto limit = overallLimit(month);
    if (!limit) return Money{};
    return *limit - overallSpent(month);
}

double BudgetService::overallProgress(YearMonth month) const {
    const auto limit = overallLimit(month);
    if (!limit || !limit->isPositive()) return 0.0;
    return static_cast<double>(overallSpent(month).minor()) / limit->minor();
}

QVector<BudgetService::CategoryBudgetStatus> BudgetService::categoryStatus(
    YearMonth month, const QVector<Category>& categories) const {
    const auto limits = budgetRepo_.categoryLimits(month);
    QVector<CategoryBudgetStatus> result;
    result.reserve(limits.size());
    for (const auto& cb : limits) {
        CategoryBudgetStatus s;
        s.categoryId = cb.categoryId;
        s.limit = cb.limit;
        s.spent = txRepo_.sumByCategory(cb.categoryId, month);
        const auto it = std::ranges::find_if(categories, [&](const Category& c) {
            return c.id == cb.categoryId;
        });
        s.categoryName = it != categories.end() ? it->name : QStringLiteral("Unknown");
        result.push_back(std::move(s));
    }
    std::ranges::sort(result, [](const auto& a, const auto& b) {
        return a.progress() > b.progress();
    });
    return result;
}

} // namespace monthmint
