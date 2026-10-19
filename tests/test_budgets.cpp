#include "test_support.h"
#include <QtTest>

using namespace monthmint;
using namespace monthmint::test;

class TestBudgets : public QObject {
    Q_OBJECT

private slots:
    void setOverallBudget() {
        TestDatabase db;
        const YearMonth month{2026, 3};
        QVERIFY(db.budgetService->setOverallLimit(month, Money{100000}).ok);
        const auto limit = db.budgetService->overallLimit(month);
        QVERIFY(limit.has_value());
        QCOMPARE(*limit, Money{100000});
    }

    void clearOverallBudget() {
        TestDatabase db;
        const YearMonth month{2026, 3};
        (void)db.budgetService->setOverallLimit(month, Money{100000});
        QVERIFY(db.budgetService->setOverallLimit(month, std::nullopt).ok);
        QVERIFY(!db.budgetService->overallLimit(month).has_value());
    }

    void remainingOverallBudget() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        const YearMonth month{2026, 3};
        (void)db.budgetService->setOverallLimit(month, Money{100000});
        (void)db.txService->addExpense(Money{30000}, QDate{2026, 3, 10}, food);

        QCOMPARE(db.budgetService->overallSpent(month), Money{30000});
        QCOMPARE(db.budgetService->overallRemaining(month), Money{70000});
        QVERIFY(qFuzzyCompare(db.budgetService->overallProgress(month), 0.3));
    }

    void categoryLimitAndRemaining() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        const YearMonth month{2026, 3};
        QVERIFY(db.budgetService->setCategoryLimit(month, food, Money{20000}).ok);
        (void)db.txService->addExpense(Money{7500}, QDate{2026, 3, 10}, food);

        const auto cats = db.catRepo->all();
        const auto statuses = db.budgetService->categoryStatus(month, cats);
        QCOMPARE(statuses.size(), 1);
        QCOMPARE(statuses[0].categoryName, QStringLiteral("Food"));
        QCOMPARE(statuses[0].limit, Money{20000});
        QCOMPARE(statuses[0].spent, Money{7500});
        QCOMPARE(statuses[0].remaining(), Money{12500});
    }

    void overBudgetProgressExceedsOne() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        const YearMonth month{2026, 3};
        (void)db.budgetService->setCategoryLimit(month, food, Money{10000});
        (void)db.txService->addExpense(Money{15000}, QDate{2026, 3, 10}, food);

        const auto cats = db.catRepo->all();
        const auto statuses = db.budgetService->categoryStatus(month, cats);
        QCOMPARE(statuses.size(), 1);
        QVERIFY(statuses[0].progress() > 1.0);
        QVERIFY(statuses[0].remaining().isNegative());
        QCOMPARE(statuses[0].remaining(), Money{-5000});
    }

    void copyFromPreviousMonth() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        const YearMonth prev{2026, 2};
        const YearMonth curr{2026, 3};
        (void)db.budgetService->setOverallLimit(prev, Money{80000});
        (void)db.budgetService->setCategoryLimit(prev, food, Money{15000});

        QVERIFY(db.budgetService->copyFromPrevious(curr).ok);
        QCOMPARE(*db.budgetService->overallLimit(curr), Money{80000});

        const auto cats = db.catRepo->all();
        const auto statuses = db.budgetService->categoryStatus(curr, cats);
        QCOMPARE(statuses.size(), 1);
        QCOMPARE(statuses[0].limit, Money{15000});
    }

    void copyAcrossYearBoundary() {
        TestDatabase db;
        const YearMonth prev{2025, 12};
        const YearMonth curr{2026, 1};
        (void)db.budgetService->setOverallLimit(prev, Money{90000});
        (void)db.budgetService->copyFromPrevious(curr);
        QCOMPARE(*db.budgetService->overallLimit(curr), Money{90000});
    }

    void negativeBudgetRejected() {
        TestDatabase db;
        QVERIFY(!db.budgetService->setOverallLimit(YearMonth{2026, 3}, Money{-100}).ok);
    }
};

QTEST_MAIN(TestBudgets)
#include "test_budgets.moc"
