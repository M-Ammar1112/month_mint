#pragma once

#include "monthmint/core/date_utils.h"
#include "monthmint/core/money.h"
#include <optional>

namespace monthmint {

struct MonthlyBudget {
    YearMonth month;
    std::optional<Money> overallLimit;
};

struct CategoryBudget {
    YearMonth month;
    qint64 categoryId = 0;
    Money limit;
};

} // namespace monthmint
