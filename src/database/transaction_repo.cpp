#include "monthmint/database/transaction_repo.h"
#include "monthmint/core/date_utils.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace monthmint {

namespace {

Transaction rowToTransaction(const QSqlQuery& q) {
    Transaction t;
    t.id = q.value(0).toLongLong();
    t.type = transactionTypeFromKey(q.value(1).toString()).value_or(TransactionType::Expense);
    t.amount = Money{q.value(2).toLongLong()};
    t.date = QDate::fromString(q.value(3).toString(), Qt::ISODate);
    t.categoryId = q.value(4).toLongLong();
    t.categoryName = q.value(5).toString();
    t.description = q.value(6).toString();
    return t;
}

constexpr const char* kSelectBase =
    "SELECT t.id, t.type, t.amount_minor, t.date, t.category_id, c.name, t.description "
    "FROM transactions t JOIN categories c ON c.id = t.category_id";

} // namespace

qint64 TransactionRepo::insert(const Transaction& t) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "INSERT INTO transactions (type, amount_minor, date, category_id, description) "
        "VALUES (?, ?, ?, ?, ?)"));
    q.addBindValue(transactionTypeKey(t.type));
    q.addBindValue(static_cast<qlonglong>(t.amount.minor()));
    q.addBindValue(toIsoDate(t.date));
    q.addBindValue(t.categoryId);
    const QString desc = t.description.isEmpty() ? QStringLiteral("") : t.description;
    q.addBindValue(desc);
    if (!q.exec()) return -1;
    return q.lastInsertId().toLongLong();
}

bool TransactionRepo::update(const Transaction& t) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "UPDATE transactions SET type = ?, amount_minor = ?, date = ?, category_id = ?, description = ? "
        "WHERE id = ?"));
    q.addBindValue(transactionTypeKey(t.type));
    q.addBindValue(static_cast<qlonglong>(t.amount.minor()));
    q.addBindValue(toIsoDate(t.date));
    q.addBindValue(t.categoryId);
    q.addBindValue(t.description.isNull() ? QStringLiteral("") : t.description);
    q.addBindValue(t.id);
    return q.exec() && q.numRowsAffected() > 0;
}

bool TransactionRepo::remove(qint64 id) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral("DELETE FROM transactions WHERE id = ?"));
    q.addBindValue(id);
    return q.exec() && q.numRowsAffected() > 0;
}

std::optional<Transaction> TransactionRepo::findById(qint64 id) const {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "SELECT t.id, t.type, t.amount_minor, t.date, t.category_id, c.name, t.description "
        "FROM transactions t JOIN categories c ON c.id = t.category_id WHERE t.id = ?"));
    q.addBindValue(id);
    if (q.exec() && q.next()) return rowToTransaction(q);
    return std::nullopt;
}

QVector<Transaction> TransactionRepo::query(const TransactionFilter& filter) const {
    QStringList clauses;
    QVector<QVariant> binds;

    if (filter.month) {
        clauses << QStringLiteral("t.date >= ? AND t.date <= ?");
        binds << toIsoDate(filter.month->firstDay()) << toIsoDate(filter.month->lastDay());
    }
    if (filter.categoryId) {
        clauses << QStringLiteral("t.category_id = ?");
        binds << *filter.categoryId;
    }
    if (filter.type) {
        clauses << QStringLiteral("t.type = ?");
        binds << transactionTypeKey(*filter.type);
    }
    if (!filter.search.isEmpty()) {
        clauses << QStringLiteral("t.description LIKE ? ESCAPE '\\'");
        QString pattern = filter.search;
        pattern.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
        pattern.replace(QLatin1Char('%'), QStringLiteral("\\%"));
        pattern.replace(QLatin1Char('_'), QStringLiteral("\\_"));
        binds << QStringLiteral("%") + pattern + QStringLiteral("%");
    }

    QString sql = QString::fromLatin1(kSelectBase);
    if (!clauses.isEmpty()) sql += QStringLiteral(" WHERE ") + clauses.join(QStringLiteral(" AND "));
    sql += QStringLiteral(" ORDER BY t.date DESC, t.id DESC");
    if (filter.limit > 0) sql += QStringLiteral(" LIMIT %1").arg(filter.limit);

    QSqlQuery q{db()};
    q.prepare(sql);
    for (const auto& b : binds) q.addBindValue(b);

    QVector<Transaction> result;
    if (!q.exec()) return result;
    while (q.next()) result.push_back(rowToTransaction(q));
    return result;
}

Money TransactionRepo::sumByType(TransactionType type, YearMonth month) const {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "SELECT COALESCE(SUM(amount_minor), 0) FROM transactions "
        "WHERE type = ? AND date >= ? AND date <= ?"));
    q.addBindValue(transactionTypeKey(type));
    q.addBindValue(toIsoDate(month.firstDay()));
    q.addBindValue(toIsoDate(month.lastDay()));
    if (q.exec() && q.next()) return Money{q.value(0).toLongLong()};
    return Money{};
}

Money TransactionRepo::sumByCategory(qint64 categoryId, YearMonth month) const {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "SELECT COALESCE(SUM(amount_minor), 0) FROM transactions "
        "WHERE type = 'expense' AND category_id = ? AND date >= ? AND date <= ?"));
    q.addBindValue(categoryId);
    q.addBindValue(toIsoDate(month.firstDay()));
    q.addBindValue(toIsoDate(month.lastDay()));
    if (q.exec() && q.next()) return Money{q.value(0).toLongLong()};
    return Money{};
}

QVector<Transaction> TransactionRepo::recent(int limit) const {
    TransactionFilter f;
    f.limit = limit;
    return query(f);
}

} // namespace monthmint
