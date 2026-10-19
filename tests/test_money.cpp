#include "monthmint/core/money.h"
#include <QtTest>

using namespace monthmint;

class TestMoney : public QObject {
    Q_OBJECT

private slots:
    void defaultIsZero() {
        const Money m;
        QCOMPARE(m.minor(), 0);
        QVERIFY(m.isZero());
    }

    void addition() {
        QCOMPARE(Money{150} + Money{250}, Money{400});
    }

    void subtraction() {
        QCOMPARE(Money{500} - Money{125}, Money{375});
    }

    void negation() {
        QCOMPARE((-Money{300}), Money{-300});
        QCOMPARE((-Money{-300}), Money{300});
    }

    void comparison() {
        QVERIFY(Money{100} < Money{200});
        QVERIFY(Money{200} > Money{100});
        QCOMPARE(Money{42}, Money{42});
    }

    void absValue() {
        QCOMPARE(Money{-55}.abs(), Money{55});
        QCOMPARE(Money{55}.abs(), Money{55});
    }

    void multiplication() {
        QCOMPARE(multiply(Money{250}, 4), Money{1000});
        QCOMPARE(multiply(Money{250}, 0), Money{0});
    }

    void divideRoundingBehaviour() {
        QCOMPARE(divideRounding(Money{100}, 3), Money{33});
        QCOMPARE(divideRounding(Money{101}, 2), Money{51});
        QCOMPARE(divideRounding(Money{100}, 4), Money{25});
    }

    void noFloatingPointLoss() {
        Money total;
        for (int i = 0; i < 10; ++i) total += Money{10};
        QCOMPARE(total, Money{100});
    }

    void additionOverflowThrows() {
        const Money max = Money::max();
        QVERIFY_THROWS_EXCEPTION(MoneyOverflowError, (void)(max + Money{1}));
    }

    void subtractionOverflowThrows() {
        const Money min = Money::min();
        QVERIFY_THROWS_EXCEPTION(MoneyOverflowError, (void)(min - Money{1}));
    }

    void structuredBindingFriendly() {
        const auto sum = Money{10} + Money{20};
        QCOMPARE(sum.minor(), 30);
    }
};

QTEST_MAIN(TestMoney)
#include "test_money.moc"
