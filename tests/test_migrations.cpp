#include "monthmint/database/connection.h"
#include "monthmint/database/migrations.h"
#include "monthmint/database/transaction_repo.h"
#include "monthmint/database/category_repo.h"

#include <QTemporaryDir>
#include <QtTest>

using namespace monthmint;

class TestMigrations : public QObject {
    Q_OBJECT

private slots:
    void freshDatabaseHasSchema() {
        QTemporaryDir dir;
        Database db{dir.filePath(QStringLiteral("m.db"))};
        QVERIFY(db.isOpen());
        QCOMPARE(schemaVersion(db.handle()), 1);
    }

    void foreignKeysEnforced() {
        QTemporaryDir dir;
        Database db{dir.filePath(QStringLiteral("fk.db"))};
        auto handle = db.handle();
        TransactionRepo tx{handle};

        Transaction t;
        t.type = TransactionType::Expense;
        t.amount = Money{100};
        t.date = QDate{2026, 3, 1};
        t.categoryId = 999999; // non-existent
        // Insert should fail due to FK constraint
        QCOMPARE(tx.insert(t), qint64{-1});
    }

    void dataPersistsAcrossReopen() {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("persist.db"));
        {
            Database db{path};
            auto handle = db.handle();
            CategoryRepo cats{handle};
            cats.seedDefaults();
            TransactionRepo tx{handle};
            const auto food = cats.findByName(QStringLiteral("Food"));
            QVERIFY(food.has_value());

            Transaction t;
            t.type = TransactionType::Expense;
            t.amount = Money{4321};
            t.date = QDate{2026, 3, 10};
            t.categoryId = food->id;
            t.description = QStringLiteral("Persisted");
            QVERIFY(tx.insert(t) > 0);
        }
        {
            Database db{path};
            auto handle = db.handle();
            TransactionRepo tx{handle};
            TransactionFilter f;
            f.month = YearMonth{2026, 3};
            const auto rows = tx.query(f);
            QCOMPARE(rows.size(), 1);
            QCOMPARE(rows[0].amount, Money{4321});
            QCOMPARE(rows[0].description, QStringLiteral("Persisted"));
        }
    }

    void migrationsAreIdempotent() {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("idem.db"));
        {
            Database db{path};
            QCOMPARE(schemaVersion(db.handle()), 1);
        }
        {
            Database db{path};
            QCOMPARE(schemaVersion(db.handle()), 1);
        }
    }

    void defaultCategoriesSeededOnce() {
        QTemporaryDir dir;
        Database db{dir.filePath(QStringLiteral("cats.db"))};
        auto handle = db.handle();
        CategoryRepo cats{handle};
        const int first = cats.seedDefaults();
        QVERIFY(first > 0);
        const int second = cats.seedDefaults();
        QCOMPARE(second, 0);
    }
};

QTEST_MAIN(TestMigrations)
#include "test_migrations.moc"
