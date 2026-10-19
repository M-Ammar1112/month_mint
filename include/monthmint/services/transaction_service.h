#pragma once

#include "monthmint/core/result.h"
#include "monthmint/database/category_repo.h"
#include "monthmint/database/transaction_repo.h"
#include "monthmint/models/transaction.h"
#include <QString>

namespace monthmint {

class TransactionService {
public:
    TransactionService(TransactionRepo& txRepo, CategoryRepo& catRepo)
        : txRepo_(txRepo), catRepo_(catRepo) {}

    [[nodiscard]] ServiceResult addExpense(Money amount, QDate date, qint64 categoryId,
                                           QString description = {});
    [[nodiscard]] ServiceResult addIncome(Money amount, QDate date, qint64 categoryId,
                                          QString description = {});
    [[nodiscard]] ServiceResult addTransaction(Transaction t);
    [[nodiscard]] ServiceResult updateTransaction(const Transaction& t);
    [[nodiscard]] ServiceResult removeTransaction(qint64 id);
    [[nodiscard]] QVector<Transaction> search(const TransactionFilter& filter) const;

private:
    [[nodiscard]] ServiceResult validate(const Transaction& t) const;
    TransactionRepo& txRepo_;
    CategoryRepo& catRepo_;
};

} // namespace monthmint
