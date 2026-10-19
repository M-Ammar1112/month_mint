#include "monthmint/database/savings_repo.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace monthmint {

std::optional<Money> SavingsRepo::target(YearMonth month) const {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral("SELECT target_minor FROM savings WHERE month_key = ?"));
    q.addBindValue(month.isoKey());
    if (q.exec() && q.next()) return Money{q.value(0).toLongLong()};
    return std::nullopt;
}

bool SavingsRepo::setTarget(YearMonth month, Money target) {
    if (target.isNegative()) return false;
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "INSERT INTO savings (month_key, target_minor) VALUES (?, ?) "
        "ON CONFLICT(month_key) DO UPDATE SET target_minor = excluded.target_minor"));
    q.addBindValue(month.isoKey());
    q.addBindValue(static_cast<qlonglong>(target.minor()));
    return q.exec();
}

Money SavingsRepo::allocated(YearMonth month) const {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "SELECT COALESCE(SUM(amount_minor), 0) FROM savings_allocations WHERE month_key = ?"));
    q.addBindValue(month.isoKey());
    if (q.exec() && q.next()) return Money{q.value(0).toLongLong()};
    return Money{};
}

qint64 SavingsRepo::addAllocation(YearMonth month, Money amount, const QString& note) {
    if (!amount.isPositive()) return -1;
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "INSERT INTO savings_allocations (month_key, amount_minor, note) VALUES (?, ?, ?)"));
    q.addBindValue(month.isoKey());
    q.addBindValue(static_cast<qlonglong>(amount.minor()));
    q.addBindValue(note.isEmpty() ? QStringLiteral("") : note);
    if (!q.exec()) {
        qDebug() << "addAllocation failed:" << q.lastError().text();
        return -1;
    }
    return q.lastInsertId().toLongLong();
}

bool SavingsRepo::removeAllocation(qint64 id) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral("DELETE FROM savings_allocations WHERE id = ?"));
    q.addBindValue(id);
    return q.exec() && q.numRowsAffected() > 0;
}

} // namespace monthmint
