#pragma once

#include "monthmint/models/settings.h"
#include <QSqlDatabase>

namespace monthmint {

class SettingsRepo {
public:
    explicit SettingsRepo(QSqlDatabase db) : connectionName_(db.connectionName()) {}

    [[nodiscard]] AppSettings load() const;
    [[nodiscard]] bool save(const AppSettings& s);

public:
    [[nodiscard]] QSqlDatabase debugDb() const { return db(); }
private:
    [[nodiscard]] QString get(const QString& key, const QString& def = {}) const;
    [[nodiscard]] bool set(const QString& key, const QString& value);
    [[nodiscard]] QSqlDatabase db() const { return QSqlDatabase::database(connectionName_); }
    QString connectionName_;
};

} // namespace monthmint
