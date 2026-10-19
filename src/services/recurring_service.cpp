#include "monthmint/services/recurring_service.h"
#include "monthmint/core/date_utils.h"
#include <algorithm>

namespace monthmint {

QDate RecurringService::dueDateFor(const RecurringRule& rule, int year, int month) const {
    const QDate first{year, month, 1};
    const int maxDay = first.daysInMonth();
    const int day = std::min(rule.dayOfMonth, maxDay);
    return QDate{year, month, day};
}

ServiceResult RecurringService::addRule(RecurringRule rule) {
    if (!rule.amount.isPositive())
        return ServiceResult::failure(QStringLiteral("Amount must be greater than zero."));
    if (rule.categoryId <= 0 || !cats_.findById(rule.categoryId))
        return ServiceResult::failure(QStringLiteral("Valid category is required."));
    if (rule.dayOfMonth < 1 || rule.dayOfMonth > 31)
        return ServiceResult::failure(QStringLiteral("Day of month must be 1-31."));
    if (rule.type == TransactionType::SavingsAllocation)
        return ServiceResult::failure(QStringLiteral("Recurring rules must be income or expense."));
    const auto id = recurring_.insert(rule);
    if (id < 0) return ServiceResult::failure(QStringLiteral("Failed to save recurring rule."));
    return ServiceResult::success(id);
}

ServiceResult RecurringService::updateRule(const RecurringRule& rule) {
    if (rule.id <= 0)
        return ServiceResult::failure(QStringLiteral("Cannot update: missing rule id."));
    if (!recurring_.update(rule))
        return ServiceResult::failure(QStringLiteral("Failed to update recurring rule."));
    return ServiceResult::success(rule.id);
}

ServiceResult RecurringService::removeRule(qint64 id) {
    if (!recurring_.remove(id))
        return ServiceResult::failure(QStringLiteral("Failed to delete recurring rule."));
    return ServiceResult::success(id);
}

QVector<RecurringRule> RecurringService::allRules() const {
    return recurring_.all();
}

bool RecurringService::isAlreadyPosted(const RecurringRule& rule, const QDate& dueDate) const {
    if (!rule.lastPostedOn.isValid()) return false;
    return rule.lastPostedOn.year() == dueDate.year() &&
           rule.lastPostedOn.month() == dueDate.month();
}

QVector<RecurringPreview> RecurringService::duePreviews(const QDate& asOf) const {
    QVector<RecurringPreview> result;
    const auto rules = recurring_.activeRules();
    for (const auto& rule : rules) {
        const auto due = dueDateFor(rule, asOf.year(), asOf.month());
        if (due > asOf) continue;
        if (isAlreadyPosted(rule, due)) continue;
        RecurringPreview p;
        p.rule = rule;
        p.dueDate = due;
        p.periodKey = YearMonth{asOf.year(), asOf.month()}.isoKey();
        result.push_back(std::move(p));
    }
    return result;
}

ServiceResult RecurringService::post(const RecurringPreview& preview) {
    const auto fresh = recurring_.findById(preview.rule.id);
    if (!fresh) return ServiceResult::failure(QStringLiteral("Recurring rule not found."));
    if (isAlreadyPosted(*fresh, preview.dueDate))
        return ServiceResult::failure(QStringLiteral("Already posted for this period."));

    Transaction t;
    t.type = fresh->type;
    t.amount = fresh->amount;
    t.date = preview.dueDate;
    t.categoryId = fresh->categoryId;
    t.description = fresh->description;

    const auto txId = tx_.insert(t);
    if (txId < 0) return ServiceResult::failure(QStringLiteral("Failed to post transaction."));

    if (!recurring_.markPosted(fresh->id, preview.dueDate)) {
        (void)tx_.remove(txId);
        return ServiceResult::failure(QStringLiteral("Failed to update recurring rule."));
    }
    return ServiceResult::success(txId);
}

} // namespace monthmint
