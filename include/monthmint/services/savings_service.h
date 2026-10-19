#pragma once

#include "monthmint/core/result.h"
#include "monthmint/database/savings_repo.h"
#include <optional>

namespace monthmint {

class SavingsService {
public:
    SavingsService(SavingsRepo& repo) : repo_(repo) {}

    [[nodiscard]] ServiceResult setTarget(YearMonth month, Money target);
    [[nodiscard]] std::optional<Money> target(YearMonth month) const;
    [[nodiscard]] ServiceResult allocate(YearMonth month, Money amount, QString note = {});
    [[nodiscard]] Money allocated(YearMonth month) const;
    [[nodiscard]] ServiceResult removeAllocation(qint64 id);

private:
    SavingsRepo& repo_;
};

} // namespace monthmint
