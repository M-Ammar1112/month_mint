#include "test_support.h"
#include <QtTest>

using namespace monthmint;
using namespace monthmint::test;

class TestTransactions : public QObject {
    Q_OBJECT

private slots:
    void addExpense() {
        TestDatabase db;
        const auto cat = db.categoryId(QStringLiteral("Food"));
        const auto r = db.txService->addExpense(Money{1500}, QDate{2026, 3, 15}, cat,
                                                QStringLiteral("Groceries"));
        QVERIFY(r.ok);
        QVERIFY(r.id > 0);

        const auto stored = db.txRepo->findById(r.id);
        QVERIFY(stored.has_value());
        QCOMPARE(stored->amount, Money{1500});
        QCOMPARE(stored->type, TransactionType::Expense);
        QCOMPARE(stored->description, QStringLiteral("Groceries"));
    }

    void addIncome() {
        TestDatabase db;
        const auto cat = db.categoryId(QStringLiteral("Salary"));
        const auto r = db.txService->addIncome(Money{250000}, QDate{2026, 3, 1}, cat,
                                               QStringLiteral("March salary"));
        QVERIFY(r.ok);
        const auto stored = db.txRepo->findById(r.id);
        QVERIFY(stored.has_value());
        QCOMPARE(stored->type, TransactionType::Income);
    }

    void rejectZeroAmount() {
        TestDatabase db;
        const auto cat = db.categoryId(QStringLiteral("Food"));
        const auto r = db.txService->addExpense(Money{0}, QDate{2026, 3, 15}, cat);
        QVERIFY(!r.ok);
    }

    void rejectInvalidCategory() {
        TestDatabase db;
        const auto r = db.txService->addExpense(Money{100}, QDate{2026, 3, 15}, 99999);
        QVERIFY(!r.ok);
    }

    void updateTransaction() {
        TestDatabase db;
        qDebug() << "All categories after seed:";
        for (const auto& c : db.catRepo->all()) {
            qDebug() << "  " << c.id << c.name;
        }
        const auto cat = db.categoryId(QStringLiteral("Food"));
        qDebug() << "Food category ID:" << cat;
        const auto r = db.txService->addExpense(Money{1500}, QDate{2026, 3, 15}, cat);
        qDebug() << "addExpense r.ok:" << r.ok << "id:" << r.id << "error:" << r.error;
        QVERIFY(r.ok);

        auto tx = db.txRepo->findById(r.id);
        qDebug() << "findById after add:" << tx.has_value();
        QVERIFY(tx.has_value());
        tx->amount = Money{2000};
        tx->description = QStringLiteral("Updated");
        const auto ur = db.txService->updateTransaction(*tx);
        qDebug() << "updateTransaction ur.ok:" << ur.ok << "error:" << ur.error;
        QVERIFY(ur.ok);

        const auto after = db.txRepo->findById(r.id);
        qDebug() << "findById after update:" << after.has_value();
        QCOMPARE(after->amount, Money{2000});
        QCOMPARE(after->description, QStringLiteral("Updated"));
    }

    void deleteTransaction() {
        TestDatabase db;
        const auto cat = db.categoryId(QStringLiteral("Food"));
        const auto r = db.txService->addExpense(Money{1500}, QDate{2026, 3, 15}, cat);
        QVERIFY(r.ok);
        QVERIFY(db.txService->removeTransaction(r.id).ok);
        QVERIFY(!db.txRepo->findById(r.id).has_value());
    }

    void filterByMonth() {
        TestDatabase db;
        const auto cat = db.categoryId(QStringLiteral("Food"));
        (void)db.txService->addExpense(Money{100}, QDate{2026, 2, 28}, cat);
        (void)db.txService->addExpense(Money{200}, QDate{2026, 3, 1}, cat);
        (void)db.txService->addExpense(Money{300}, QDate{2026, 3, 31}, cat);

        TransactionFilter f;
        f.month = YearMonth{2026, 3};
        const auto rows = db.txRepo->query(f);
        QCOMPARE(rows.size(), 2);
    }

    void filterByType() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        const auto salary = db.categoryId(QStringLiteral("Salary"));
        (void)db.txService->addExpense(Money{100}, QDate{2026, 3, 1}, food);
        (void)db.txService->addIncome(Money{5000}, QDate{2026, 3, 1}, salary);

        TransactionFilter f;
        f.month = YearMonth{2026, 3};
        f.type = TransactionType::Income;
        QCOMPARE(db.txRepo->query(f).size(), 1);
    }

    void searchDescription() {
        TestDatabase db;
        const auto cat = db.categoryId(QStringLiteral("Food"));
        (void)db.txService->addExpense(Money{100}, QDate{2026, 3, 1}, cat, QStringLiteral("Coffee beans"));
        (void)db.txService->addExpense(Money{200}, QDate{2026, 3, 2}, cat, QStringLiteral("Lunch"));

        TransactionFilter f;
        f.month = YearMonth{2026, 3};
        f.search = QStringLiteral("coffee");
        QCOMPARE(db.txRepo->query(f).size(), 1);
    }
};

QTEST_MAIN(TestTransactions)
#include "test_transactions.moc"
