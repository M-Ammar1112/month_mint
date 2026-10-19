#pragma once

#include "monthmint/core/money.h"
#include "monthmint/models/transaction.h"
#include <QDate>
#include <QString>

namespace monthmint {

struct RecurringRule {
    qint64 id = 0;
    TransactionType type = TransactionType::Expense;
    Money amount;
    qint64 categoryId = 0;
    QString categoryName;
    QString description;
    int dayOfMonth = 1;        // 1..28 safe default; clamped for shorter months
    bool active = true;
    QDate lastPostedOn;        // invalid if never posted
};

struct RecurringPreview {
    RecurringRule rule;
    QDate dueDate;
    QString periodKey;         // "YYYY-MM"
};

} // namespace monthmint
