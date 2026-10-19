#pragma once

#include "monthmint/core/result.h"
#include "monthmint/database/category_repo.h"
#include "monthmint/database/recurring_repo.h"
#include "monthmint/database/transaction_repo.h"
#include "monthmint/models/recurring.h"
#include <QDate>
#include <QVector>

namespace monthmint {

class RecurringService {
public:
    RecurringService(RecurringRepo& recurringRepo, TransactionRepo& txRepo, CategoryRepo& catRepo)
        : recurring_(recurringRepo), tx_(txRepo), cats_(catRepo) {}

    [[nodiscard]] ServiceResult addRule(RecurringRule rule);
    [[nodiscard]] ServiceResult updateRule(const RecurringRule& rule);
    [[nodiscard]] ServiceResult removeRule(qint64 id);
    [[nodiscard]] QVector<RecurringRule> allRules() const;
    [[nodiscard]] QVector<RecurringPreview> duePreviews(const QDate& asOf) const;
    [[nodiscard]] ServiceResult post(const RecurringPreview& preview);
    [[nodiscard]] bool isAlreadyPosted(const RecurringRule& rule, const QDate& dueDate) const;

private:
    [[nodiscard]] QDate dueDateFor(const RecurringRule& rule, int year, int month) const;
    RecurringRepo& recurring_;
    TransactionRepo& tx_;
    CategoryRepo& cats_;
};

} // namespace monthmint
