#include "monthmint/database/settings_repo.h"

#include <QSqlQuery>
#include <QVariant>

namespace monthmint {

QString SettingsRepo::get(const QString& key, const QString& def) const {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral("SELECT value FROM settings WHERE key = ?"));
    q.addBindValue(key);
    if (q.exec() && q.next()) return q.value(0).toString();
    return def;
}

bool SettingsRepo::set(const QString& key, const QString& value) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "INSERT INTO settings (key, value) VALUES (?, ?) "
        "ON CONFLICT(key) DO UPDATE SET value = excluded.value"));
    q.addBindValue(key);
    q.addBindValue(value);
    return q.exec();
}

AppSettings SettingsRepo::load() const {
    AppSettings s;
    s.currencyCode = get(QStringLiteral("currency"), QStringLiteral("USD"));
    s.localeTag = get(QStringLiteral("locale"));
    s.theme = get(QStringLiteral("theme"), QStringLiteral("light"));
    s.firstRunCompleted = get(QStringLiteral("first_run_completed")) == QLatin1String("1");
    return s;
}

bool SettingsRepo::save(const AppSettings& s) {
    bool ok = true;
    ok &= set(QStringLiteral("currency"), s.currencyCode);
    ok &= set(QStringLiteral("locale"), s.localeTag);
    ok &= set(QStringLiteral("theme"), s.theme);
    ok &= set(QStringLiteral("first_run_completed"), s.firstRunCompleted ? QStringLiteral("1") : QStringLiteral("0"));
    return ok;
}

} // namespace monthmint
