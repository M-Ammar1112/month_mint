#include "test_support.h"
#include <QtTest>

using namespace monthmint;
using namespace monthmint::test;

class TestSummary : public QObject {
    Q_OBJECT

private slots:
    void monthlyIncomeAndExpenses() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        const auto salary = db.categoryId(QStringLiteral("Salary"));
        const YearMonth month{2026, 3};

        (void)db.txService->addIncome(Money{300000}, QDate{2026, 3, 1}, salary);
        (void)db.txService->addExpense(Money{50000}, QDate{2026, 3, 5}, food);
        (void)db.txService->addExpense(Money{25000}, QDate{2026, 3, 20}, food);

        const auto s = db.summaryService->monthlySummary(month);
        QCOMPARE(s.income, Money{300000});
        QCOMPARE(s.expenses, Money{75000});
        QCOMPARE(s.netCashFlow(), Money{225000});
    }

    void transactionsOutsideMonthExcluded() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        (void)db.txService->addExpense(Money{1000}, QDate{2026, 2, 28}, food);
        (void)db.txService->addExpense(Money{2000}, QDate{2026, 4, 1}, food);
        (void)db.txService->addExpense(Money{3000}, QDate{2026, 3, 15}, food);

        const auto s = db.summaryService->monthlySummary(YearMonth{2026, 3});
        QCOMPARE(s.expenses, Money{3000});
    }

    void spendingByCategory() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        const auto housing = db.categoryId(QStringLiteral("Housing"));

        (void)db.txService->addExpense(Money{4000}, QDate{2026, 3, 1}, food);
        (void)db.txService->addExpense(Money{10000}, QDate{2026, 3, 2}, housing);

        const auto cats = db.catRepo->all();
        const auto spends = db.summaryService->spendingByCategory(YearMonth{2026, 3}, cats);
        QCOMPARE(spends.size(), 2);
        // Sorted descending by amount
        QCOMPARE(spends[0].name, QStringLiteral("Housing"));
        QCOMPARE(spends[0].spent, Money{10000});
        QVERIFY(qFuzzyCompare(spends[0].share, 10000.0 / 14000.0));
    }

    void savingsNotCountedAsExpenses() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        const YearMonth month{2026, 3};

        (void)db.txService->addExpense(Money{5000}, QDate{2026, 3, 5}, food);
        (void)db.savingsService->setTarget(month, Money{20000});
        (void)db.savingsService->allocate(month, Money{10000});

        const auto s = db.summaryService->monthlySummary(month);
        QCOMPARE(s.expenses, Money{5000});       // allocation excluded
        QCOMPARE(s.savingsAllocated, Money{10000});
        QCOMPARE(s.savingsTarget, Money{20000});
        QVERIFY(qFuzzyCompare(s.savingsProgress(), 0.5));
    }

    void emptyMonthIsZero() {
        TestDatabase db;
        const auto s = db.summaryService->monthlySummary(YearMonth{2026, 5});
        QCOMPARE(s.income, Money{0});
        QCOMPARE(s.expenses, Money{0});
        QCOMPARE(s.netCashFlow(), Money{0});
    }

    void savingsProgressZeroWhenNoTarget() {
        TestDatabase db;
        const auto s = db.summaryService->monthlySummary(YearMonth{2026, 3});
        QVERIFY(qFuzzyCompare(s.savingsProgress(), 0.0));
    }
};

QTEST_MAIN(TestSummary)
#include "test_summary.moc"
