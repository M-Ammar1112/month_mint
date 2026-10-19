#include "test_support.h"
#include "monthmint/core/formatting.h"
#include <QtTest>
#include <QSqlQuery>
#include <QDebug>

using namespace monthmint;
using namespace monthmint::test;

int main() {
    QTemporaryDir dir;
    TestDatabase db;

    auto food = db.categoryId("Food");
    qDebug() << "Food category ID:" << food;
    qDebug() << "All categories:";
    for (const auto& c : db.catRepo->all()) {
        qDebug() << "  " << c.id << c.name << int(c.kind);
    }

    auto r = db.txService->addExpense(Money{1500}, QDate{2026, 3, 15}, food);
    qDebug() << "Add expense result:" << r.ok << r.id << r.error;

    auto tx = db.txRepo->findById(r.id);
    qDebug() << "Find by ID:" << (tx.has_value() ? "found" : "not found");

    if (tx) {
        tx->amount = Money{2000};
        tx->description = "Updated";
        auto ur = db.txService->updateTransaction(*tx);
        qDebug() << "Update result:" << ur.ok << ur.error;

        auto after = db.txRepo->findById(r.id);
        qDebug() << "After update find:" << (after.has_value() ? "found" : "not found");
    }

    TransactionFilter f;
    f.month = YearMonth{2026, 3};
    auto rows = db.txRepo->query(f);
    qDebug() << "Query March:" << rows.size() << "rows";

    // Also check raw SQL
    auto rawDb = db.txRepo->debugDb();
    QSqlQuery q(rawDb);
    q.exec("SELECT * FROM transactions");
    while (q.next()) {
        qDebug() << "Raw row:" << q.value(0).toLongLong() << q.value(1).toString()
                 << q.value(2).toLongLong() << q.value(3).toString();
    }

    return 0;
}
