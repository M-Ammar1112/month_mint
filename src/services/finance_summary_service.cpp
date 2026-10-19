#include "monthmint/services/finance_summary_service.h"
#include <QHash>
#include <algorithm>

namespace monthmint {

MonthlySummary FinanceSummaryService::monthlySummary(YearMonth month) const {
    MonthlySummary s;
    s.month = month;
    s.income = txRepo_.sumByType(TransactionType::Income, month);
    s.expenses = txRepo_.sumByType(TransactionType::Expense, month);
    s.savingsTarget = savingsRepo_.target(month).value_or(Money{});
    s.savingsAllocated = savingsRepo_.allocated(month);
    return s;
}

QVector<CategorySpend> FinanceSummaryService::spendingByCategory(
    YearMonth month, const QVector<Category>& categories) const {
    const auto expenses = txRepo_.query(TransactionFilter{
        month, std::nullopt, TransactionType::Expense, {}, 0});

    QVector<CategorySpend> result;
    Money total;
    for (const auto& tx : expenses) total += tx.amount;

    QHash<qint64, Money> byCat;
    for (const auto& tx : expenses) byCat[tx.categoryId] += tx.amount;

    result.reserve(byCat.size());
    for (auto it = byCat.begin(); it != byCat.end(); ++it) {
        CategorySpend cs;
        cs.categoryId = it.key();
        cs.spent = it.value();
        const auto catIt = std::ranges::find_if(categories, [&](const Category& c) {
            return c.id == cs.categoryId;
        });
        cs.name = catIt != categories.end() ? catIt->name : QStringLiteral("Unknown");
        cs.share = total.isPositive()
                       ? static_cast<double>(cs.spent.minor()) / total.minor()
                       : 0.0;
        result.push_back(std::move(cs));
    }

    std::ranges::sort(result, [](const CategorySpend& a, const CategorySpend& b) {
        return a.spent > b.spent;
    });
    return result;
}

} // namespace monthmint
