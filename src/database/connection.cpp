#include "monthmint/database/connection.h"
#include "monthmint/database/migrations.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>
#include <QDebug>

namespace monthmint {

struct Database::Impl {
    QString connectionName;
    QString dbPath;
    bool open = false;
};

Database::Database(const QString& path) : impl_(std::make_unique<Impl>()) {
    impl_->dbPath = path.isEmpty() ? defaultDatabasePath() : path;

    const QFileInfo fi{impl_->dbPath};
    QDir().mkpath(fi.absolutePath());

    impl_->connectionName = QStringLiteral("monthmint_%1")
                                .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), impl_->connectionName);
    db.setDatabaseName(impl_->dbPath);
    if (!db.open()) {
        qWarning() << "MonthMint: failed to open database:" << db.lastError().text();
        return;
    }

    QSqlQuery pragma{db};
    pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
    pragma.exec(QStringLiteral("PRAGMA journal_mode = WAL"));
    pragma.exec(QStringLiteral("PRAGMA synchronous = NORMAL"));

    migrateDatabase(db);
    impl_->open = true;
}

Database::~Database() {
    if (impl_) {
        QSqlDatabase::removeDatabase(impl_->connectionName);
    }
}

Database::Database(Database&&) noexcept = default;
Database& Database::operator=(Database&&) noexcept = default;

bool Database::isOpen() const { return impl_ && impl_->open; }
QString Database::path() const { return impl_ ? impl_->dbPath : QString{}; }

QSqlDatabase Database::handle() const {
    return impl_ ? QSqlDatabase::database(impl_->connectionName) : QSqlDatabase{};
}

QString Database::defaultDataDirectory() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return base;
}

QString Database::defaultDatabasePath() {
    return defaultDataDirectory() + QStringLiteral("/monthmint.db");
}

} // namespace monthmint
