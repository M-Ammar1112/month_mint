#include "monthmint/models/transaction.h"

namespace monthmint {

QString transactionTypeLabel(TransactionType t) {
    switch (t) {
        case TransactionType::Expense: return QStringLiteral("Expense");
        case TransactionType::Income: return QStringLiteral("Income");
        case TransactionType::SavingsAllocation: return QStringLiteral("Savings");
    }
    return {};
}

QString transactionTypeKey(TransactionType t) {
    switch (t) {
        case TransactionType::Expense: return QStringLiteral("expense");
        case TransactionType::Income: return QStringLiteral("income");
        case TransactionType::SavingsAllocation: return QStringLiteral("savings");
    }
    return {};
}

std::optional<TransactionType> transactionTypeFromKey(const QString& key) {
    if (key == QLatin1String("expense")) return TransactionType::Expense;
    if (key == QLatin1String("income")) return TransactionType::Income;
    if (key == QLatin1String("savings")) return TransactionType::SavingsAllocation;
    return std::nullopt;
}

} // namespace monthmint
