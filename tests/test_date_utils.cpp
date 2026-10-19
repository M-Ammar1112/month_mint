#include "monthmint/core/date_utils.h"
#include <QtTest>

using namespace monthmint;

class TestDateUtils : public QObject {
    Q_OBJECT

private slots:
    void previousMonthWithinYear() {
        auto r = previousMonth(YearMonth{2026, 5});
        auto e = YearMonth{2026, 4};
        QCOMPARE(r, e);
    }

    void previousMonthAcrossYear() {
        auto r = previousMonth(YearMonth{2026, 1});
        auto e = YearMonth{2025, 12};
        QCOMPARE(r, e);
    }

    void nextMonthWithinYear() {
        auto r = nextMonth(YearMonth{2026, 5});
        auto e = YearMonth{2026, 6};
        QCOMPARE(r, e);
    }

    void nextMonthAcrossYear() {
        auto r = nextMonth(YearMonth{2026, 12});
        auto e = YearMonth{2027, 1};
        QCOMPARE(r, e);
    }

    void monthBoundaries() {
        const YearMonth feb{2024, 2}; // leap year
        const QDate d1(2024, 2, 1);
        QCOMPARE(feb.firstDay(), d1);
        const QDate d2(2024, 2, 29);
        QCOMPARE(feb.lastDay(), d2);

        const YearMonth febNonLeap{2025, 2};
        const QDate d3(2025, 2, 28);
        QCOMPARE(febNonLeap.lastDay(), d3);
    }

    void clampToMonthRange() {
        const YearMonth jan{2026, 1};
        const QDate expected1(2026, 1, 1);
        QCOMPARE(jan.clamp(QDate{2025, 12, 31}), expected1);
        const QDate expected2(2026, 1, 31);
        QCOMPARE(jan.clamp(QDate{2026, 3, 1}), expected2);
        const QDate expected3(2026, 1, 15);
        QCOMPARE(jan.clamp(QDate{2026, 1, 15}), expected3);
    }

    void yearBoundaryTraversal() {
        YearMonth m{2026, 11};
        m = nextMonth(m);
        auto expected = YearMonth{2026, 12};
        QCOMPARE(m, expected);
        m = nextMonth(m);
        expected = YearMonth{2027, 1};
        QCOMPARE(m, expected);
        m = previousMonth(m);
        expected = YearMonth{2026, 12};
        QCOMPARE(m, expected);
    }

    void isoKeyFormat() {
        const QString s1 = YearMonth{2026, 3}.isoKey();
        QCOMPARE(s1, QStringLiteral("2026-03"));
        const QString s2 = YearMonth{2026, 12}.isoKey();
        QCOMPARE(s2, QStringLiteral("2026-12"));
    }

    void labelFormat() {
        const QString s = YearMonth{2026, 1}.label();
        QCOMPARE(s, QStringLiteral("January 2026"));
    }
};

QTEST_MAIN(TestDateUtils)
#include "test_date_utils.moc"
