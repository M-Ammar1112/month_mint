#include "monthmint/database/category_repo.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace monthmint {

namespace {
Category rowToCategory(const QSqlQuery& q) {
    Category c;
    c.id = q.value(0).toLongLong();
    c.name = q.value(1).toString();
    c.kind = categoryKindFromKey(q.value(2).toString());
    c.isDefault = q.value(3).toBool();
    return c;
}
} // namespace

qint64 CategoryRepo::insert(const Category& c) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "INSERT INTO categories (name, kind, is_default) VALUES (?, ?, ?)"));
    q.addBindValue(c.name);
    q.addBindValue(categoryKindKey(c.kind));
    q.addBindValue(c.isDefault ? 1 : 0);
    if (!q.exec()) return -1;
    return q.lastInsertId().toLongLong();
}

bool CategoryRepo::update(const Category& c) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "UPDATE categories SET name = ?, kind = ?, is_default = ? WHERE id = ?"));
    q.addBindValue(c.name);
    q.addBindValue(categoryKindKey(c.kind));
    q.addBindValue(c.isDefault ? 1 : 0);
    q.addBindValue(c.id);
    return q.exec() && q.numRowsAffected() > 0;
}

bool CategoryRepo::remove(qint64 id) {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral("DELETE FROM categories WHERE id = ?"));
    q.addBindValue(id);
    return q.exec() && q.numRowsAffected() > 0;
}

std::optional<Category> CategoryRepo::findById(qint64 id) const {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral("SELECT id, name, kind, is_default FROM categories WHERE id = ?"));
    q.addBindValue(id);
    if (q.exec() && q.next()) return rowToCategory(q);
    return std::nullopt;
}

std::optional<Category> CategoryRepo::findByName(const QString& name) const {
    QSqlQuery q{db()};
    q.prepare(QStringLiteral(
        "SELECT id, name, kind, is_default FROM categories WHERE LOWER(name) = LOWER(?)"));
    q.addBindValue(name);
    if (q.exec() && q.next()) return rowToCategory(q);
    return std::nullopt;
}

QVector<Category> CategoryRepo::all() const {
    QSqlQuery q{db()};
    q.exec(QStringLiteral(
        "SELECT id, name, kind, is_default FROM categories ORDER BY name COLLATE NOCASE"));
    QVector<Category> result;
    while (q.next()) result.push_back(rowToCategory(q));
    return result;
}

int CategoryRepo::seedDefaults() {
    if (!all().isEmpty()) return 0;
    int seeded = 0;
    const auto insertAll = [&](const QVector<Category>& cats) {
        for (auto c : cats) {
            c.isDefault = true;
            if (insert(c) > 0) ++seeded;
        }
    };
    insertAll(defaultExpenseCategories());
    // "Other" already seeded as expense; add income-only categories, skipping duplicates
    for (auto c : defaultIncomeCategories()) {
        if (findByName(c.name)) continue;
        c.isDefault = true;
        if (insert(c) > 0) ++seeded;
    }
    return seeded;
}

} // namespace monthmint
