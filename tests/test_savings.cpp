#include "test_support.h"
#include <QtTest>

using namespace monthmint;
using namespace monthmint::test;

class TestSavings : public QObject {
    Q_OBJECT

private slots:
    void setAndReadTarget() {
        TestDatabase db;
        const YearMonth month{2026, 3};
        QVERIFY(db.savingsService->setTarget(month, Money{50000}).ok);
        const auto t = db.savingsService->target(month);
        QVERIFY(t.has_value());
        QCOMPARE(*t, Money{50000});
    }

    void allocateAccumulates() {
        TestDatabase db;
        const YearMonth month{2026, 3};
        auto r1 = db.savingsService->allocate(month, Money{10000});
        qDebug() << "alloc1:" << r1.ok << r1.id << r1.error;
        auto r2 = db.savingsService->allocate(month, Money{15000});
        qDebug() << "alloc2:" << r2.ok << r2.id << r2.error;
        const auto alloc = db.savingsService->allocated(month);
        qDebug() << "allocated:" << alloc.minor();
        QCOMPARE(alloc, Money{25000});
    }

    void removeAllocation() {
        TestDatabase db;
        const YearMonth month{2026, 3};
        const auto r = db.savingsService->allocate(month, Money{10000});
        qDebug() << "alloc for remove:" << r.ok << r.id << r.error;
        QVERIFY(r.ok);
        const auto removeRes = db.savingsService->removeAllocation(r.id);
        qDebug() << "remove:" << removeRes.ok << removeRes.error;
        QVERIFY(removeRes.ok);
        QCOMPARE(db.savingsService->allocated(month), Money{0});
    }

    void allocationNotExpense() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        const YearMonth month{2026, 3};
        (void)db.txService->addExpense(Money{5000}, QDate{2026, 3, 5}, food);
        (void)db.savingsService->allocate(month, Money{30000});

        const auto s = db.summaryService->monthlySummary(month);
        QCOMPARE(s.expenses, Money{5000});
    }

    void allocationsIsolatedByMonth() {
        TestDatabase db;
        (void)db.savingsService->allocate(YearMonth{2026, 3}, Money{10000});
        (void)db.savingsService->allocate(YearMonth{2026, 4}, Money{20000});
        QCOMPARE(db.savingsService->allocated(YearMonth{2026, 3}), Money{10000});
        QCOMPARE(db.savingsService->allocated(YearMonth{2026, 4}), Money{20000});
    }

    void negativeTargetRejected() {
        TestDatabase db;
        QVERIFY(!db.savingsService->setTarget(YearMonth{2026, 3}, Money{-1}).ok);
    }

    void allocationMustBePositive() {
        TestDatabase db;
        QVERIFY(!db.savingsService->allocate(YearMonth{2026, 3}, Money{0}).ok);
    }
};

QTEST_MAIN(TestSavings)
#include "test_savings.moc"
