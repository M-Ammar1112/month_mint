#include "monthmint/services/transaction_service.h"

namespace monthmint {

ServiceResult TransactionService::validate(const Transaction& t) const {
    if (!t.amount.isPositive())
        return ServiceResult::failure(QStringLiteral("Amount must be greater than zero."));
    if (!t.date.isValid())
        return ServiceResult::failure(QStringLiteral("Date is invalid."));
    if (t.categoryId <= 0)
        return ServiceResult::failure(QStringLiteral("Category is required."));
    if (!catRepo_.findById(t.categoryId))
        return ServiceResult::failure(QStringLiteral("Selected category does not exist."));
    return ServiceResult::success();
}

ServiceResult TransactionService::addTransaction(Transaction t) {
    if (auto v = validate(t); !v.ok) return v;
    const auto id = txRepo_.insert(t);
    if (id < 0) return ServiceResult::failure(QStringLiteral("Failed to save transaction."));
    return ServiceResult::success(id);
}

ServiceResult TransactionService::addExpense(Money amount, QDate date, qint64 categoryId,
                                             QString description) {
    Transaction t;
    t.type = TransactionType::Expense;
    t.amount = amount;
    t.date = date;
    t.categoryId = categoryId;
    t.description = std::move(description);
    return addTransaction(std::move(t));
}

ServiceResult TransactionService::addIncome(Money amount, QDate date, qint64 categoryId,
                                            QString description) {
    Transaction t;
    t.type = TransactionType::Income;
    t.amount = amount;
    t.date = date;
    t.categoryId = categoryId;
    t.description = std::move(description);
    return addTransaction(std::move(t));
}

ServiceResult TransactionService::updateTransaction(const Transaction& t) {
    if (t.id <= 0)
        return ServiceResult::failure(QStringLiteral("Cannot update: missing transaction id."));
    if (auto v = validate(t); !v.ok) return v;
    if (!txRepo_.update(t))
        return ServiceResult::failure(QStringLiteral("Failed to update transaction."));
    return ServiceResult::success(t.id);
}

ServiceResult TransactionService::removeTransaction(qint64 id) {
    if (id <= 0)
        return ServiceResult::failure(QStringLiteral("Cannot delete: missing transaction id."));
    if (!txRepo_.remove(id))
        return ServiceResult::failure(QStringLiteral("Failed to delete transaction."));
    return ServiceResult::success(id);
}

QVector<Transaction> TransactionService::search(const TransactionFilter& filter) const {
    return txRepo_.query(filter);
}

} // namespace monthmint
