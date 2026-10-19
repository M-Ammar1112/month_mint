#include "test_support.h"
#include <QtTest>

using namespace monthmint;
using namespace monthmint::test;

class TestRecurring : public QObject {
    Q_OBJECT

private slots:
    void addRuleAndList() {
        TestDatabase db;
        const auto rent = db.categoryId(QStringLiteral("Housing"));
        RecurringRule r;
        r.type = TransactionType::Expense;
        r.amount = Money{50000};
        r.categoryId = rent;
        r.description = QStringLiteral("Rent");
        r.dayOfMonth = 1;

        const auto result = db.recurringService->addRule(r);
        QVERIFY(result.ok);
        QCOMPARE(db.recurringService->allRules().size(), 1);
    }

    void duePreviewAppearsWhenDue() {
        TestDatabase db;
        const auto rent = db.categoryId(QStringLiteral("Housing"));
        RecurringRule r;
        r.type = TransactionType::Expense;
        r.amount = Money{50000};
        r.categoryId = rent;
        r.dayOfMonth = 1;
        (void)db.recurringService->addRule(r);

        const auto previews = db.recurringService->duePreviews(QDate{2026, 3, 15});
        QCOMPARE(previews.size(), 1);
        auto expectedDate = QDate(2026, 3, 1);
        QCOMPARE(previews[0].dueDate, expectedDate);
    }

    void notDueBeforeDay() {
        TestDatabase db;
        const auto rent = db.categoryId(QStringLiteral("Housing"));
        RecurringRule r;
        r.type = TransactionType::Expense;
        r.amount = Money{50000};
        r.categoryId = rent;
        r.dayOfMonth = 25;
        (void)db.recurringService->addRule(r);

        const auto previews = db.recurringService->duePreviews(QDate{2026, 3, 10});
        QVERIFY(previews.isEmpty());
    }

    void postingCreatesTransaction() {
        TestDatabase db;
        const auto rent = db.categoryId(QStringLiteral("Housing"));
        RecurringRule r;
        r.type = TransactionType::Expense;
        r.amount = Money{50000};
        r.categoryId = rent;
        r.dayOfMonth = 1;
        (void)db.recurringService->addRule(r);

        const auto previews = db.recurringService->duePreviews(QDate{2026, 3, 15});
        QCOMPARE(previews.size(), 1);
        const auto postResult = db.recurringService->post(previews[0]);
        QVERIFY(postResult.ok);

        const auto s = db.summaryService->monthlySummary(YearMonth{2026, 3});
        QCOMPARE(s.expenses, Money{50000});
    }

    void duplicatePreventedWithinSamePeriod() {
        TestDatabase db;
        const auto rent = db.categoryId(QStringLiteral("Housing"));
        RecurringRule r;
        r.type = TransactionType::Expense;
        r.amount = Money{50000};
        r.categoryId = rent;
        r.dayOfMonth = 1;
        (void)db.recurringService->addRule(r);

        const auto previews = db.recurringService->duePreviews(QDate{2026, 3, 15});
        QCOMPARE(previews.size(), 1);
        QVERIFY(db.recurringService->post(previews[0]).ok);

        // No longer due in the same month
        QVERIFY(db.recurringService->duePreviews(QDate{2026, 3, 20}).isEmpty());

        // Direct repost of the same preview is rejected
        QVERIFY(!db.recurringService->post(previews[0]).ok);
    }

    void dueAgainNextMonth() {
        TestDatabase db;
        const auto rent = db.categoryId(QStringLiteral("Housing"));
        RecurringRule r;
        r.type = TransactionType::Expense;
        r.amount = Money{50000};
        r.categoryId = rent;
        r.dayOfMonth = 1;
        (void)db.recurringService->addRule(r);

        const auto march = db.recurringService->duePreviews(QDate{2026, 3, 15});
        QCOMPARE(march.size(), 1);
        (void)db.recurringService->post(march[0]);

        const auto april = db.recurringService->duePreviews(QDate{2026, 4, 15});
        QCOMPARE(april.size(), 1);
        const QDate expected(2026, 4, 1);
        QCOMPARE(april[0].dueDate, expected);
    }

    void dayClampedForShortMonths() {
        TestDatabase db;
        const auto rent = db.categoryId(QStringLiteral("Housing"));
        RecurringRule r;
        r.type = TransactionType::Expense;
        r.amount = Money{50000};
        r.categoryId = rent;
        r.dayOfMonth = 31;
        (void)db.recurringService->addRule(r);

        const auto previews = db.recurringService->duePreviews(QDate{2026, 2, 28});
        QCOMPARE(previews.size(), 1);
        const QDate expected(2026, 2, 28);
        QCOMPARE(previews[0].dueDate, expected);
    }

    void incomeRecurringPosted() {
        TestDatabase db;
        const auto salary = db.categoryId(QStringLiteral("Salary"));
        RecurringRule r;
        r.type = TransactionType::Income;
        r.amount = Money{300000};
        r.categoryId = salary;
        r.dayOfMonth = 5;
        (void)db.recurringService->addRule(r);

        const auto previews = db.recurringService->duePreviews(QDate{2026, 3, 10});
        QCOMPARE(previews.size(), 1);
        QVERIFY(db.recurringService->post(previews[0]).ok);
        const auto s = db.summaryService->monthlySummary(YearMonth{2026, 3});
        QCOMPARE(s.income, Money{300000});
        QCOMPARE(s.expenses, Money{0});
    }
};

QTEST_MAIN(TestRecurring)
#include "test_recurring.moc"
