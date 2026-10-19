#include "monthmint/ui/app_context.h"
#include "monthmint/ui/first_run_dialog.h"
#include "monthmint/ui/main_window.h"
#include "monthmint/ui/theme.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>

using namespace monthmint;

namespace {

/// Applies a pending restore (.restore file) before the DB is opened.
void applyPendingRestore(const QString& dbPath) {
    const QString pending = dbPath + QStringLiteral(".restore");
    if (!QFile::exists(pending)) return;
    if (QFile::exists(dbPath)) QFile::remove(dbPath);
    QFile::rename(pending, dbPath);
}

} // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("MonthMint"));
    QApplication::setOrganizationName(QStringLiteral("MonthMint"));
    QApplication::setApplicationVersion(QStringLiteral(MONTHMINT_VERSION));

    const QString dbPath = Database::defaultDatabasePath();
    applyPendingRestore(dbPath);

    auto ctx = std::make_unique<ui::AppContext>(dbPath);
    if (!ctx->isOpen()) {
        QMessageBox::critical(nullptr, QStringLiteral("MonthMint"),
                              QStringLiteral("Could not open the database at:\n%1").arg(dbPath));
        return 1;
    }

    // First-run setup
    if (!ctx->settings.firstRunCompleted) {
        ui::FirstRunDialog setup;
        setup.exec();
        const auto choices = setup.choices();
        ctx->settings.currencyCode = choices.currencyCode;
        ctx->settings.theme = choices.theme;
        ctx->settings.firstRunCompleted = true;
        (void)ctx->settingsRepo.save(ctx->settings);

        const auto month = yearMonthOf(QDate::currentDate());
        if (choices.monthlyBudgetMinor) {
            (void)ctx->budgetService.setOverallLimit(month, Money{*choices.monthlyBudgetMinor});
        }
        if (choices.savingsTargetMinor) {
            (void)ctx->savingsService.setTarget(month, Money{*choices.savingsTargetMinor});
        }
    }

    ui::applyTheme(ctx->settings.theme);

    ui::MainWindow window(std::move(ctx));
    window.show();

    return app.exec();
}
