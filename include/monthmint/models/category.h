#pragma once

#include <QString>
#include <QVector>

namespace monthmint {

enum class CategoryKind {
    Expense,
    Income,
    Both,
};

struct Category {
    qint64 id = 0;
    QString name;
    CategoryKind kind = CategoryKind::Expense;
    bool isDefault = false;

    [[nodiscard]] bool operator==(const Category&) const = default;
};

[[nodiscard]] QVector<Category> defaultExpenseCategories();
[[nodiscard]] QVector<Category> defaultIncomeCategories();
[[nodiscard]] QString categoryKindKey(CategoryKind k);
[[nodiscard]] CategoryKind categoryKindFromKey(const QString& key, CategoryKind fallback = CategoryKind::Expense);

} // namespace monthmint
