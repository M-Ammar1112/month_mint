#include "test_support.h"
#include "monthmint/core/csv.h"

#include <QFile>
#include <QTextStream>
#include <QtTest>

using namespace monthmint;
using namespace monthmint::test;

class TestCsvExport : public QObject {
    Q_OBJECT

private slots:
    void escapeSimple() {
        QCOMPARE(csvEscape(QStringLiteral("hello")), QStringLiteral("hello"));
    }

    void escapeComma() {
        QCOMPARE(csvEscape(QStringLiteral("a,b")), QStringLiteral("\"a,b\""));
    }

    void escapeQuotes() {
        QCOMPARE(csvEscape(QStringLiteral("say \"hi\"")), QStringLiteral("\"say \"\"hi\"\"\""));
    }

    void rowJoin() {
        QCOMPARE(toCsvRow({QStringLiteral("a"), QStringLiteral("b,c")}),
                 QStringLiteral("a,\"b,c\""));
    }

    void parseSimple() {
        const auto fields = parseCsvRow(QStringLiteral("a,b,c"));
        QCOMPARE(fields.size(), 3);
        QCOMPARE(fields[0], QStringLiteral("a"));
        QCOMPARE(fields[2], QStringLiteral("c"));
    }

    void parseQuoted() {
        const auto fields = parseCsvRow(QStringLiteral("a,\"b,c\",d"));
        QCOMPARE(fields.size(), 3);
        QCOMPARE(fields[1], QStringLiteral("b,c"));
    }

    void parseEscapedQuotes() {
        const auto fields = parseCsvRow(QStringLiteral("\"say \"\"hi\"\"\",x"));
        QCOMPARE(fields.size(), 2);
        QCOMPARE(fields[0], QStringLiteral("say \"hi\""));
    }

    void exportIncludesHeaderAndRows() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        db.txService->addExpense(Money{1234}, QDate{2026, 3, 10}, food, QStringLiteral("Groceries"));
        db.txService->addExpense(Money{5678}, QDate{2026, 3, 12}, food, QStringLiteral("Lunch"));

        const QString csvPath = db.dir.filePath(QStringLiteral("export.csv"));
        QString error;
        QVERIFY(db.exportService->exportToCsv(TransactionFilter{}, csvPath, &error));

        QFile file{csvPath};
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
        QTextStream in{&file};
        const auto header = in.readLine();
        QVERIFY(header.contains(QStringLiteral("Date")));
        QVERIFY(header.contains(QStringLiteral("AmountMinor")));

        QString body;
        while (!in.atEnd()) body += in.readLine() + QLatin1Char('\n');

        // Both amounts appear regardless of ordering.
        QVERIFY(body.contains(QStringLiteral("1234")));
        QVERIFY(body.contains(QStringLiteral("5678")));
        QVERIFY(body.contains(QStringLiteral("Groceries")));
        QVERIFY(body.contains(QStringLiteral("Lunch")));
    }

    void exportDescriptionWithCommaIsQuoted() {
        TestDatabase db;
        const auto food = db.categoryId(QStringLiteral("Food"));
        db.txService->addExpense(Money{100}, QDate{2026, 3, 10}, food,
                                 QStringLiteral("Coffee, milk"));
        const QString csvPath = db.dir.filePath(QStringLiteral("export2.csv"));
        QVERIFY(db.exportService->exportToCsv(TransactionFilter{}, csvPath));

        QFile file{csvPath};
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
        QTextStream in{&file};
        in.readLine(); // header
        const auto dataLine = in.readLine();
        QVERIFY(dataLine.contains(QStringLiteral("\"Coffee, milk\"")));
        QCOMPARE(parseCsvRow(dataLine).size(), 5);
    }
};

QTEST_MAIN(TestCsvExport)
#include "test_csv_export.moc"
