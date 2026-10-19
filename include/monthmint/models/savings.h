#pragma once

#include "monthmint/core/date_utils.h"
#include "monthmint/core/money.h"
#include <optional>

namespace monthmint {

struct SavingsTarget {
    YearMonth month;
    Money target;
};

struct SavingsAllocation {
    qint64 id = 0;
    YearMonth month;
    Money amount;
    QString note;
};

} // namespace monthmint
