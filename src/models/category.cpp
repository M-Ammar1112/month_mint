#include "monthmint/models/category.h"

namespace monthmint {

QVector<Category> defaultExpenseCategories() {
    const QStringList names = {
        QStringLiteral("Food"),
        QStringLiteral("Housing"),
        QStringLiteral("Bills"),
        QStringLiteral("Transportation"),
        QStringLiteral("Shopping"),
        QStringLiteral("Health"),
        QStringLiteral("Entertainment"),
        QStringLiteral("Education"),
        QStringLiteral("Other"),
    };
    QVector<Category> result;
    result.reserve(names.size());
    for (const auto& n : names) {
        result.push_back(Category{0, n, CategoryKind::Expense, true});
    }
    return result;
}

QVector<Category> defaultIncomeCategories() {
    const QStringList names = {
        QStringLiteral("Salary"),
        QStringLiteral("Freelancing"),
        QStringLiteral("Business"),
        QStringLiteral("Gifts"),
        QStringLiteral("Other"),
    };
    QVector<Category> result;
    result.reserve(names.size());
    for (const auto& n : names) {
        result.push_back(Category{0, n, CategoryKind::Income, true});
    }
    return result;
}

QString categoryKindKey(CategoryKind k) {
    switch (k) {
        case CategoryKind::Expense: return QStringLiteral("expense");
        case CategoryKind::Income: return QStringLiteral("income");
        case CategoryKind::Both: return QStringLiteral("both");
    }
    return {};
}

CategoryKind categoryKindFromKey(const QString& key, CategoryKind fallback) {
    if (key == QLatin1String("expense")) return CategoryKind::Expense;
    if (key == QLatin1String("income")) return CategoryKind::Income;
    if (key == QLatin1String("both")) return CategoryKind::Both;
    return fallback;
}

} // namespace monthmint
