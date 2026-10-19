#pragma once

#include "monthmint/models/category.h"
#include <QSqlDatabase>
#include <QVector>
#include <optional>

namespace monthmint {

class CategoryRepo {
public:
    explicit CategoryRepo(QSqlDatabase db) : connectionName_(db.connectionName()) {}

    [[nodiscard]] qint64 insert(const Category& c);
    [[nodiscard]] bool update(const Category& c);
    [[nodiscard]] bool remove(qint64 id);
    [[nodiscard]] std::optional<Category> findById(qint64 id) const;
    [[nodiscard]] std::optional<Category> findByName(const QString& name) const;
    [[nodiscard]] QVector<Category> all() const;
    [[nodiscard]] int seedDefaults();

private:
    [[nodiscard]] QSqlDatabase db() const { return QSqlDatabase::database(connectionName_); }
public:
    [[nodiscard]] QSqlDatabase debugDb() const { return db(); }
    QString connectionName_;
};

} // namespace monthmint
