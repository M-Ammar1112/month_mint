#include "monthmint/services/backup_service.h"

#include <QFile>
#include <QFileInfo>

namespace monthmint {

bool BackupService::backupDatabase(const QString& sourcePath, const QString& destinationPath,
                                   QString* errorOut) {
    if (!QFileInfo::exists(sourcePath)) {
        if (errorOut) *errorOut = QStringLiteral("Source database not found.");
        return false;
    }
    if (QFileInfo::exists(destinationPath)) QFile::remove(destinationPath);
    if (!QFile::copy(sourcePath, destinationPath)) {
        if (errorOut) *errorOut = QStringLiteral("Failed to copy database file.");
        return false;
    }
    return true;
}

bool BackupService::restoreDatabase(const QString& backupPath, const QString& destinationPath,
                                    QString* errorOut) {
    if (!QFileInfo::exists(backupPath)) {
        if (errorOut) *errorOut = QStringLiteral("Backup file not found.");
        return false;
    }
    if (QFileInfo::exists(destinationPath)) {
        if (!QFile::remove(destinationPath)) {
            if (errorOut) *errorOut = QStringLiteral("Failed to remove existing database.");
            return false;
        }
    }
    if (!QFile::copy(backupPath, destinationPath)) {
        if (errorOut) *errorOut = QStringLiteral("Failed to copy backup file.");
        return false;
    }
    return true;
}

} // namespace monthmint
