#pragma once

#include "monthmint/core/money.h"
#include <QDate>
#include <QString>
#include <optional>

namespace monthmint {

enum class TransactionType {
    Expense,
    Income,
    SavingsAllocation,
};

[[nodiscard]] QString transactionTypeLabel(TransactionType t);
[[nodiscard]] QString transactionTypeKey(TransactionType t);
[[nodiscard]] std::optional<TransactionType> transactionTypeFromKey(const QString& key);

struct Transaction {
    qint64 id = 0;
    TransactionType type = TransactionType::Expense;
    Money amount;                       // always stored as positive magnitude
    QDate date;
    qint64 categoryId = 0;
    QString categoryName;               // denormalized for display
    QString description;

    [[nodiscard]] bool isValid() const {
        return amount.isPositive() && date.isValid() && categoryId > 0;
    }
};

} // namespace monthmint
