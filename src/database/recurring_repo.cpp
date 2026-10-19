#include "monthmint/database/recurring_repo.h"
#include "monthmint/core/date_utils.h"

#include <QSqlQuery>
#include <QVariant>

namespace monthmint {

namespace {
RecurringRule rowToRule(const QSqlQuery& q) {
    RecurringRule r;
    r.id = q.value(0).toLongLong();
    r.type = transactionTypeFromKey(q.value(1).toString()).value_or(TransactionType::Expense);
    r.amount = Money{q.value(2).toLongLong()};
    r.categoryId = q.value(3).toLongLong();
    r.categoryName = q.value(4).toString();
    r.description = q.value(5).toString();
    r.dayOfMonth = q.value(6).toInt();
    r.active = q.value(7).toBool();
    const auto last = q.value(8).toString();
    r.lastPostedOn = last.isEmpty() ? QDate{} : QDate::fromString(last, Qt::ISODate);
    return r;
}

constexpr const char* kSelectBase =
    "SELECT r.id, r.type, r.amount_minor, r.category_id, c.name, r.description, "
    "r.day_of_month, r.active, r.last_posted_on "
    "FROM recurring_rules r JOIN categories c ON c.id = r.category_id";
} // namespace

qint64 RecurringRepo::insert(const RecurringRule& r) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "INSERT INTO recurring_rules (type, amount_minor, category_id, description, "
        "day_of_month, active, last_posted_on) VALUES (?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(transactionTypeKey(r.type));
    q.addBindValue(static_cast<qlonglong>(r.amount.minor()));
    q.addBindValue(r.categoryId);
    q.addBindValue(r.description);
    q.addBindValue(r.dayOfMonth);
    q.addBindValue(r.active ? 1 : 0);
    q.addBindValue(r.lastPostedOn.isValid() ? toIsoDate(r.lastPostedOn) : QVariant{});
    if (!q.exec()) return -1;
    return q.lastInsertId().toLongLong();
}

bool RecurringRepo::update(const RecurringRule& r) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "UPDATE recurring_rules SET type = ?, amount_minor = ?, category_id = ?, "
        "description = ?, day_of_month = ?, active = ?, last_posted_on = ? WHERE id = ?"));
    q.addBindValue(transactionTypeKey(r.type));
    q.addBindValue(static_cast<qlonglong>(r.amount.minor()));
    q.addBindValue(r.categoryId);
    q.addBindValue(r.description);
    q.addBindValue(r.dayOfMonth);
    q.addBindValue(r.active ? 1 : 0);
    q.addBindValue(r.lastPostedOn.isValid() ? toIsoDate(r.lastPostedOn) : QVariant{});
    q.addBindValue(r.id);
    return q.exec() && q.numRowsAffected() > 0;
}

bool RecurringRepo::remove(qint64 id) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral("DELETE FROM recurring_rules WHERE id = ?"));
    q.addBindValue(id);
    return q.exec() && q.numRowsAffected() > 0;
}

std::optional<RecurringRule> RecurringRepo::findById(qint64 id) const {
    QSqlQuery q{db()};
    q.prepare(QString::fromLatin1(kSelectBase) + QStringLiteral(" WHERE r.id = ?"));
    q.addBindValue(id);
    if (q.exec() && q.next()) return rowToRule(q);
    return std::nullopt;
}

QVector<RecurringRule> RecurringRepo::all() const {
    QSqlQuery q{db()};
    q.prepare(QString::fromLatin1(kSelectBase) + QStringLiteral(" ORDER BY r.day_of_month, r.id"));
    QVector<RecurringRule> result;
    if (!q.exec()) return result;
    while (q.next()) result.push_back(rowToRule(q));
    return result;
}

QVector<RecurringRule> RecurringRepo::activeRules() const {
    QSqlQuery q{db()};
    q.prepare(QString::fromLatin1(kSelectBase) +
               QStringLiteral(" WHERE r.active = 1 ORDER BY r.day_of_month, r.id"));
    QVector<RecurringRule> result;
    if (!q.exec()) return result;
    while (q.next()) result.push_back(rowToRule(q));
    return result;
}

bool RecurringRepo::markPosted(qint64 ruleId, const QDate& date) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "UPDATE recurring_rules SET last_posted_on = ? WHERE id = ?"));
    q.addBindValue(toIsoDate(date));
    q.addBindValue(ruleId);
    return q.exec() && q.numRowsAffected() > 0;
}

} // namespace monthmint
