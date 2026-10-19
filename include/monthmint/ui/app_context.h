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
#include <memory>

namespace monthmint::ui {

/// Owns the database and wires repositories/services for the UI layer.
struct AppContext {
    explicit AppContext(const QString& dbPath = {})
        : database(dbPath),
          txRepo(database.handle()),
          categoryRepo(database.handle()),
          budgetRepo(database.handle()),
          savingsRepo(database.handle()),
          recurringRepo(database.handle()),
          settingsRepo(database.handle()),
          transactionService(txRepo, categoryRepo),
          budgetService(budgetRepo, txRepo),
          summaryService(txRepo, savingsRepo),
          recurringService(recurringRepo, txRepo, categoryRepo),
          savingsService(savingsRepo),
          exportService(txRepo) {
        (void)categoryRepo.seedDefaults();
        settings = settingsRepo.load();
    }

    [[nodiscard]] bool isOpen() const { return database.isOpen(); }

    Database database;
    TransactionRepo txRepo;
    CategoryRepo categoryRepo;
    BudgetRepo budgetRepo;
    SavingsRepo savingsRepo;
    RecurringRepo recurringRepo;
    SettingsRepo settingsRepo;

    TransactionService transactionService;
    BudgetService budgetService;
    FinanceSummaryService summaryService;
    RecurringService recurringService;
    SavingsService savingsService;
    ExportService exportService;

    AppSettings settings;
};

} // namespace monthmint::ui
