#pragma once

#include "monthmint/core/date_utils.h"
#include "monthmint/core/money.h"
#include "monthmint/database/savings_repo.h"
#include "monthmint/database/transaction_repo.h"
#include "monthmint/models/category.h"
#include "monthmint/models/savings.h"
#include <QVector>

namespace monthmint {

struct MonthlySummary {
    YearMonth month;
    Money income;
    Money expenses;
    Money netCashFlow() const { return income - expenses; }
    Money savingsTarget;
    Money savingsAllocated;
    double savingsProgress() const {
        return savingsTarget.isPositive()
                   ? static_cast<double>(savingsAllocated.minor()) / savingsTarget.minor()
                   : 0.0;
    }
};

struct CategorySpend {
    qint64 categoryId = 0;
    QString name;
    Money spent;
    double share = 0.0; // fraction of total expenses
};

class FinanceSummaryService {
public:
    FinanceSummaryService(TransactionRepo& txRepo, SavingsRepo& savingsRepo)
        : txRepo_(txRepo), savingsRepo_(savingsRepo) {}

    [[nodiscard]] MonthlySummary monthlySummary(YearMonth month) const;
    [[nodiscard]] QVector<CategorySpend> spendingByCategory(YearMonth month,
                                                            const QVector<Category>& categories) const;

private:
    TransactionRepo& txRepo_;
    SavingsRepo& savingsRepo_;
};

} // namespace monthmint
