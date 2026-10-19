#include "monthmint/services/savings_service.h"

namespace monthmint {

ServiceResult SavingsService::setTarget(YearMonth month, Money target) {
    if (target.isNegative())
        return ServiceResult::failure(QStringLiteral("Savings target cannot be negative."));
    if (!repo_.setTarget(month, target))
        return ServiceResult::failure(QStringLiteral("Failed to save savings target."));
    return ServiceResult::success();
}

std::optional<Money> SavingsService::target(YearMonth month) const {
    return repo_.target(month);
}

ServiceResult SavingsService::allocate(YearMonth month, Money amount, QString note) {
    if (!amount.isPositive())
        return ServiceResult::failure(QStringLiteral("Allocation amount must be positive."));
    const auto id = repo_.addAllocation(month, amount, note);
    if (id < 0) return ServiceResult::failure(QStringLiteral("Failed to record savings allocation."));
    return ServiceResult::success(id);
}

Money SavingsService::allocated(YearMonth month) const {
    return repo_.allocated(month);
}

ServiceResult SavingsService::removeAllocation(qint64 id) {
    if (!repo_.removeAllocation(id))
        return ServiceResult::failure(QStringLiteral("Failed to remove allocation."));
    return ServiceResult::success(id);
}

} // namespace monthmint
