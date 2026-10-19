#pragma once

#include "monthmint/database/budget_repo.h"
#include "monthmint/database/category_repo.h"
#include "monthmint/database/connection.h"
#include "monthmint/database/recurring_repo.h"
#include "monthmint/database/savings_repo.h"
#include "monthmint/database/settings_repo.h"
#include "monthmint/database/transaction_repo.h"
#include "monthmint/services/budget_service.h"
#include "monthmint/services/export_service.h"
#include "monthmint/services/finance_summary_service.h"
#include "monthmint/services/recurring_service.h"
#include "monthmint/services/savings_service.h"
#include "monthmint/services/transaction_service.h"

#include <QTemporaryDir>
#include <QString>
#include <memory>

namespace monthmint::test {

/// Creates an isolated temporary SQLite database with default categories seeded.
struct TestDatabase {
    QTemporaryDir dir;

    std::unique_ptr<Database> database;
    std::unique_ptr<TransactionRepo> txRepo;
    std::unique_ptr<CategoryRepo> catRepo;
    std::unique_ptr<BudgetRepo> budgetRepo;
    std::unique_ptr<SavingsRepo> savingsRepo;
    std::unique_ptr<RecurringRepo> recurringRepo;
    std::unique_ptr<SettingsRepo> settingsRepo;
    std::unique_ptr<TransactionService> txService;
    std::unique_ptr<BudgetService> budgetService;
    std::unique_ptr<FinanceSummaryService> summaryService;
    std::unique_ptr<RecurringService> recurringService;
    std::unique_ptr<SavingsService> savingsService;
    std::unique_ptr<ExportService> exportService;

    TestDatabase() {
        const QString path = dir.filePath(QStringLiteral("test.db"));
        database = std::make_unique<Database>(path);
        auto db = database->handle();
        txRepo = std::make_unique<TransactionRepo>(db);
        catRepo = std::make_unique<CategoryRepo>(db);
        budgetRepo = std::make_unique<BudgetRepo>(db);
        savingsRepo = std::make_unique<SavingsRepo>(db);
        recurringRepo = std::make_unique<RecurringRepo>(db);
        settingsRepo = std::make_unique<SettingsRepo>(db);
        txService = std::make_unique<TransactionService>(*txRepo, *catRepo);
        budgetService = std::make_unique<BudgetService>(*budgetRepo, *txRepo);
        summaryService = std::make_unique<FinanceSummaryService>(*txRepo, *savingsRepo);
        recurringService = std::make_unique<RecurringService>(*recurringRepo, *txRepo, *catRepo);
        savingsService = std::make_unique<SavingsService>(*savingsRepo);
        exportService = std::make_unique<ExportService>(*txRepo);
        catRepo->seedDefaults();
    }

    [[nodiscard]] qint64 categoryId(const QString& name) const {
        const auto c = catRepo->findByName(name);
        return c ? c->id : 0;
    }

    [[nodiscard]] QString path() const { return database->path(); }
};

} // namespace monthmint::test
