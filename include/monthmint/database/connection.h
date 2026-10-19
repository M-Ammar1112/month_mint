#pragma once

#include <QSqlDatabase>
#include <QString>
#include <memory>

namespace monthmint {

/// RAII wrapper around a named SQLite connection.
class Database {
public:
    /// Opens (or creates) the SQLite database at `path` and applies migrations.
    /// Pass an empty path to use the OS application-data location.
    explicit Database(const QString& path = {});
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) noexcept;
    Database& operator=(Database&&) noexcept;

    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] QString path() const;
    [[nodiscard]] QSqlDatabase handle() const;

    [[nodiscard]] static QString defaultDataDirectory();
    [[nodiscard]] static QString defaultDatabasePath();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace monthmint
