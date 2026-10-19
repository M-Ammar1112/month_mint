#include "monthmint/database/migrations.h"

#include <QSqlQuery>
#include <QVariant>
#include <QVector>
#include <functional>

namespace monthmint {

namespace {

struct Migration {
    int version;
    std::function<void(QSqlDatabase&)> apply;
};

const QVector<Migration>& migrations() {
    static const QVector<Migration> list = {
        {1, [](QSqlDatabase& db) {
            QSqlQuery q{db};
            q.exec(QStringLiteral(
                "CREATE TABLE IF NOT EXISTS schema_meta ("
                "  key TEXT PRIMARY KEY,"
                "  value TEXT NOT NULL"
                ")"));
            q.exec(QStringLiteral(
                "CREATE TABLE categories ("
                "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "  name TEXT NOT NULL UNIQUE,"
                "  kind TEXT NOT NULL CHECK (kind IN ('expense','income','both')),"
                "  is_default INTEGER NOT NULL DEFAULT 0"
                ")"));
            q.exec(QStringLiteral(
                "CREATE TABLE transactions ("
                "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "  type TEXT NOT NULL CHECK (type IN ('expense','income','savings')),"
                "  amount_minor INTEGER NOT NULL CHECK (amount_minor > 0),"
                "  date TEXT NOT NULL,"
                "  category_id INTEGER NOT NULL REFERENCES categories(id) ON DELETE RESTRICT,"
                "  description TEXT NOT NULL DEFAULT ''"
                ")"));
            q.exec(QStringLiteral("CREATE INDEX idx_tx_date ON transactions(date)"));
            q.exec(QStringLiteral("CREATE INDEX idx_tx_category ON transactions(category_id)"));
            q.exec(QStringLiteral("CREATE INDEX idx_tx_type_date ON transactions(type, date)"));

            q.exec(QStringLiteral(
                "CREATE TABLE monthly_budgets ("
                "  month_key TEXT NOT NULL,"
                "  overall_limit_minor INTEGER,"
                "  PRIMARY KEY (month_key)"
                ")"));
            q.exec(QStringLiteral(
                "CREATE TABLE category_budgets ("
                "  month_key TEXT NOT NULL,"
                "  category_id INTEGER NOT NULL REFERENCES categories(id) ON DELETE CASCADE,"
                "  limit_minor INTEGER NOT NULL CHECK (limit_minor >= 0),"
                "  PRIMARY KEY (month_key, category_id)"
                ")"));

            q.exec(QStringLiteral(
                "CREATE TABLE savings ("
                "  month_key TEXT NOT NULL,"
                "  target_minor INTEGER NOT NULL CHECK (target_minor >= 0),"
                "  PRIMARY KEY (month_key)"
                ")"));
            q.exec(QStringLiteral(
                "CREATE TABLE savings_allocations ("
                "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "  month_key TEXT NOT NULL,"
                "  amount_minor INTEGER NOT NULL CHECK (amount_minor > 0),"
                "  note TEXT NOT NULL DEFAULT ''"
                ")"));
            q.exec(QStringLiteral("CREATE INDEX idx_savings_alloc_month ON savings_allocations(month_key)"));

            q.exec(QStringLiteral(
                "CREATE TABLE recurring_rules ("
                "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "  type TEXT NOT NULL CHECK (type IN ('expense','income')),"
                "  amount_minor INTEGER NOT NULL CHECK (amount_minor > 0),"
                "  category_id INTEGER NOT NULL REFERENCES categories(id) ON DELETE RESTRICT,"
                "  description TEXT NOT NULL DEFAULT '',"
                "  day_of_month INTEGER NOT NULL CHECK (day_of_month BETWEEN 1 AND 31),"
                "  active INTEGER NOT NULL DEFAULT 1,"
                "  last_posted_on TEXT"
                ")"));

            q.exec(QStringLiteral(
                "CREATE TABLE settings ("
                "  key TEXT PRIMARY KEY,"
                "  value TEXT NOT NULL"
                ")"));
        }},
    };
    return list;
}

void setVersion(QSqlDatabase& db, int v) {
    QSqlQuery q{db};
    q.prepare(QStringLiteral(
        "INSERT INTO schema_meta (key, value) VALUES ('schema_version', ?) "
        "ON CONFLICT(key) DO UPDATE SET value = excluded.value"));
    q.addBindValue(QString::number(v));
    q.exec();
}

} // namespace

int schemaVersion(QSqlDatabase db) {
    QSqlQuery q{db};
    q.exec(QStringLiteral("SELECT value FROM schema_meta WHERE key = 'schema_version'"));
    if (q.next()) return q.value(0).toInt();
    return 0;
}

int migrateDatabase(QSqlDatabase db) {
    // Ensure meta table exists even before first migration runs
    {
        QSqlQuery q{db};
        q.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS schema_meta ("
            "  key TEXT PRIMARY KEY,"
            "  value TEXT NOT NULL"
            ")"));
    }

    int current = schemaVersion(db);
    for (const auto& m : migrations()) {
        if (m.version > current) {
            m.apply(db);
            setVersion(db, m.version);
            current = m.version;
        }
    }
    return current;
}

} // namespace monthmint
