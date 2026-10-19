#pragma once

#include <QSqlDatabase>

namespace monthmint {

/// Applies pending schema migrations. Returns the final schema version.
int migrateDatabase(QSqlDatabase db);

/// Current schema version stored in the database (0 if none).
int schemaVersion(QSqlDatabase db);

} // namespace monthmint
