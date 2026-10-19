#include "monthmint/core/formatting.h"
#include <QtTest>

using namespace monthmint;

class TestFormatting : public QObject {
    Q_OBJECT

private slots:
    void formatUsd() {
        const auto s = formatMoney(Money{123456}, QStringLiteral("USD"));
        QVERIFY(s.contains(QStringLiteral("1,234")));
        QVERIFY(s.contains(QStringLiteral(".56")));
    }

    void formatPkrNoDecimals() {
        const auto s = formatMoney(Money{5000}, QStringLiteral("PKR"));
        QVERIFY(s.contains(QStringLiteral("5,000")));
        QVERIFY(!s.contains(QLatin1Char('.')));
    }

    void formatNegative() {
        const auto s = formatMoney(Money{-1050}, QStringLiteral("USD"));
        QVERIFY(s.startsWith(QLatin1Char('-')));
    }

    void parseSimpleUsd() {
        const auto m = parseMoney(QStringLiteral("12.34"), QStringLiteral("USD"));
        QVERIFY(m.has_value());
        QCOMPARE(*m, Money{1234});
    }

    void parseWithThousands() {
        const auto m = parseMoney(QStringLiteral("1,234.56"), QStringLiteral("USD"));
        QVERIFY(m.has_value());
        QCOMPARE(*m, Money{123456});
    }

    void parseSingleDecimal() {
        const auto m = parseMoney(QStringLiteral("5.5"), QStringLiteral("USD"));
        QVERIFY(m.has_value());
        QCOMPARE(*m, Money{550});
    }

    void parsePkrInteger() {
        const auto m = parseMoney(QStringLiteral("2500"), QStringLiteral("PKR"));
        QVERIFY(m.has_value());
        QCOMPARE(*m, Money{2500});
    }

    void parseEmptyFails() {
        QVERIFY(!parseMoney(QString(), QStringLiteral("USD")).has_value());
    }

    void parseGarbageFails() {
        QVERIFY(!parseMoney(QStringLiteral("abc"), QStringLiteral("USD")).has_value());
    }

    void roundTrip() {
        const Money original{98765};
        const auto text = formatMoney(original, QStringLiteral("USD"));
        const auto parsed = parseMoney(text, QStringLiteral("USD"));
        QVERIFY(parsed.has_value());
        QCOMPARE(*parsed, original);
    }
};

QTEST_MAIN(TestFormatting)
#include "test_formatting.moc"
