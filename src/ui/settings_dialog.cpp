#include "monthmint/ui/settings_dialog.h"
#include "monthmint/core/formatting.h"
#include "monthmint/services/backup_service.h"
#include "monthmint/ui/app_context.h"
#include "monthmint/ui/theme.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace monthmint::ui {

SettingsDialog::SettingsDialog(AppContext& ctx, QWidget* parent)
    : QDialog(parent), ctx_(ctx) {
    setWindowTitle(QStringLiteral("Settings"));
    setMinimumWidth(460);
    buildUi();
}

void SettingsDialog::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(16);

    auto* title = new QLabel(QStringLiteral("Settings"), this);
    title->setObjectName(QStringLiteral("Title"));
    root->addWidget(title);

    // Preferences
    auto* prefs = new QGroupBox(QStringLiteral("Preferences"), this);
    auto* form = new QFormLayout(prefs);
    form->setSpacing(10);

    currencyCombo_ = new QComboBox(prefs);
    for (const auto& code : availableCurrencyCodes()) {
        const auto& info = currencyInfo(code);
        currencyCombo_->addItem(QStringLiteral("%1 (%2) — %3").arg(code, info.symbol, info.name), code);
    }
    currencyCombo_->setCurrentIndex(currencyCombo_->findData(ctx_.settings.currencyCode));

    localeCombo_ = new QComboBox(prefs);
    localeCombo_->addItem(QStringLiteral("System default"), QString{});
    for (const auto& loc : {QStringLiteral("en_US"), QStringLiteral("en_GB"), QStringLiteral("en_PK"),
                            QStringLiteral("de_DE"), QStringLiteral("fr_FR")}) {
        localeCombo_->addItem(loc, loc);
    }
    localeCombo_->setCurrentIndex(localeCombo_->findData(ctx_.settings.localeTag));

    themeCombo_ = new QComboBox(prefs);
    themeCombo_->addItem(QStringLiteral("Light"), QStringLiteral("light"));
    themeCombo_->addItem(QStringLiteral("Dark"), QStringLiteral("dark"));
    themeCombo_->setCurrentIndex(themeCombo_->findData(ctx_.settings.theme));

    form->addRow(QStringLiteral("Currency"), currencyCombo_);
    form->addRow(QStringLiteral("Locale"), localeCombo_);
    form->addRow(QStringLiteral("Theme"), themeCombo_);
    root->addWidget(prefs);

    // Data management
    auto* data = new QGroupBox(QStringLiteral("Data"), this);
    auto* dataLayout = new QVBoxLayout(data);
    auto* exportBtn = new QPushButton(QStringLiteral("Export transactions to CSV…"), data);
    exportBtn->setObjectName(QStringLiteral("SecondaryButton"));
    auto* backupBtn = new QPushButton(QStringLiteral("Back up database…"), data);
    backupBtn->setObjectName(QStringLiteral("SecondaryButton"));
    auto* restoreBtn = new QPushButton(QStringLiteral("Restore from backup…"), data);
    restoreBtn->setObjectName(QStringLiteral("SecondaryButton"));
    dataLayout->addWidget(exportBtn);
    dataLayout->addWidget(backupBtn);
    dataLayout->addWidget(restoreBtn);
    auto* dbPathLabel = new QLabel(QStringLiteral("Database: %1").arg(ctx_.database.path()), data);
    dbPathLabel->setObjectName(QStringLiteral("Subtitle"));
    dbPathLabel->setWordWrap(true);
    dataLayout->addWidget(dbPathLabel);
    root->addWidget(data);

    root->addStretch();

    auto* buttons = new QDialogButtonBox(this);
    auto* cancel = buttons->addButton(QStringLiteral("Cancel"), QDialogButtonBox::RejectRole);
    cancel->setObjectName(QStringLiteral("SecondaryButton"));
    auto* saveBtn = buttons->addButton(QStringLiteral("Save"), QDialogButtonBox::AcceptRole);
    saveBtn->setObjectName(QStringLiteral("PrimaryButton"));
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::save);
    connect(exportBtn, &QPushButton::clicked, this, &SettingsDialog::exportCsv);
    connect(backupBtn, &QPushButton::clicked, this, &SettingsDialog::backup);
    connect(restoreBtn, &QPushButton::clicked, this, &SettingsDialog::restore);
    root->addWidget(buttons);
}

void SettingsDialog::save() {
    ctx_.settings.currencyCode = currencyCombo_->currentData().toString();
    ctx_.settings.localeTag = localeCombo_->currentData().toString();
    ctx_.settings.theme = themeCombo_->currentData().toString();
    ctx_.settingsRepo.save(ctx_.settings);
    applyTheme(ctx_.settings.theme);
    emit settingsChanged();
    accept();
}

void SettingsDialog::exportCsv() {
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("Export transactions"), QStringLiteral("monthmint_transactions.csv"),
        QStringLiteral("CSV files (*.csv)"));
    if (path.isEmpty()) return;

    TransactionFilter filter;
    QString error;
    if (!ctx_.exportService.exportToCsv(filter, path, &error)) {
        QMessageBox::warning(this, QStringLiteral("Export failed"), error);
        return;
    }
    QMessageBox::information(this, QStringLiteral("Export complete"),
                             QStringLiteral("Transactions exported to:\n%1").arg(path));
}

void SettingsDialog::backup() {
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("Back up database"), QStringLiteral("monthmint_backup.db"),
        QStringLiteral("SQLite database (*.db)"));
    if (path.isEmpty()) return;
    QString error;
    if (!BackupService::backupDatabase(ctx_.database.path(), path, &error)) {
        QMessageBox::warning(this, QStringLiteral("Backup failed"), error);
        return;
    }
    QMessageBox::information(this, QStringLiteral("Backup complete"),
                             QStringLiteral("Database backed up to:\n%1").arg(path));
}

void SettingsDialog::restore() {
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Restore from backup"), {}, QStringLiteral("SQLite database (*.db)"));
    if (path.isEmpty()) return;

    const auto answer = QMessageBox::question(
        this, QStringLiteral("Restore database"),
        QStringLiteral("Restoring will replace your current data. The application must restart.\n\n"
                       "Continue?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    QString error;
    // Write to a pending file; applied on next startup to avoid an open-handle conflict.
    const QString pending = ctx_.database.path() + QStringLiteral(".restore");
    if (!BackupService::restoreDatabase(path, pending, &error)) {
        QMessageBox::warning(this, QStringLiteral("Restore failed"), error);
        return;
    }
    QMessageBox::information(
        this, QStringLiteral("Restore scheduled"),
        QStringLiteral("The backup will be applied the next time MonthMint starts. "
                       "Please close and reopen the application."));
}

} // namespace monthmint::ui
