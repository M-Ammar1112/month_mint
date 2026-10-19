#pragma once

#include <QString>

namespace monthmint {

class BackupService {
public:
    /// Copies the live database file to `destinationPath` (SQLite backup-safe copy).
    [[nodiscard]] static bool backupDatabase(const QString& sourcePath,
                                             const QString& destinationPath,
                                             QString* errorOut = nullptr);

    /// Restores from backup. The live DB must not be open; caller closes/reopens.
    [[nodiscard]] static bool restoreDatabase(const QString& backupPath,
                                              const QString& destinationPath,
                                              QString* errorOut = nullptr);
};

} // namespace monthmint
