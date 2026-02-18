#include <QtTest/QtTest>

#include "core/utils/TsTranslator.h"
#include "core/utils/TranslationManager.h"

#include <QCoreApplication>
#include <QTemporaryDir>

static const QByteArray kValidTs =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE TS>\n"
    "<TS version=\"2.1\" language=\"fr\" sourcelanguage=\"en\">\n"
    "<context>\n"
    "    <name>MainWindow</name>\n"
    "    <message>\n"
    "        <source>Settings</source>\n"
    "        <translation>Paramètres</translation>\n"
    "    </message>\n"
    "    <message>\n"
    "        <source>Close</source>\n"
    "        <translation type=\"unfinished\"></translation>\n"
    "    </message>\n"
    "    <message>\n"
    "        <source>Open</source>\n"
    "        <translation type=\"unfinished\">Ouvrir</translation>\n"
    "    </message>\n"
    "    <message numerus=\"yes\">\n"
    "        <source>%n mod(s) found</source>\n"
    "        <translation>\n"
    "            <numerusform>%n mod trouvé</numerusform>\n"
    "            <numerusform>%n mods trouvés</numerusform>\n"
    "        </translation>\n"
    "    </message>\n"
    "</context>\n"
    "</TS>\n";

class TestTsTranslator : public QObject {
    Q_OBJECT

private slots:
    void testLoadValidFile();
    void testLanguageAttribute();
    void testBasicLookup();
    void testUnfinishedEmptySkipped();
    void testUnfinishedWithTextKept();
    void testPluralSingular();
    void testPluralPlural();
    void testUnknownContextReturnsEmpty();
    void testUnknownSourceReturnsEmpty();
    void testEmptyAfterConstruction();
    void testInvalidFileReturnsFalse();
    void testLanguageDiscovery();
};

void TestTsTranslator::testLoadValidFile() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("test.ts");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(kValidTs);
    f.close();

    TsTranslator t;
    QVERIFY(t.loadTsFile(path));
    QVERIFY(!t.isEmpty());
}

void TestTsTranslator::testLanguageAttribute() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("test.ts");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(kValidTs);
    f.close();

    TsTranslator t;
    t.loadTsFile(path);
    QCOMPARE(t.language(), QStringLiteral("fr"));
}

void TestTsTranslator::testBasicLookup() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("test.ts");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(kValidTs);
    f.close();

    TsTranslator t;
    t.loadTsFile(path);
    QString result = t.translate("MainWindow", "Settings");
    QCOMPARE(result, QString::fromUtf8("Paramètres"));
}

void TestTsTranslator::testUnfinishedEmptySkipped() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("test.ts");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(kValidTs);
    f.close();

    TsTranslator t;
    t.loadTsFile(path);
    // "Close" is unfinished with empty text -> should not be in map
    QString result = t.translate("MainWindow", "Close");
    QVERIFY(result.isEmpty());
}

void TestTsTranslator::testUnfinishedWithTextKept() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("test.ts");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(kValidTs);
    f.close();

    TsTranslator t;
    t.loadTsFile(path);
    // "Open" is unfinished but has text -> should be kept
    QString result = t.translate("MainWindow", "Open");
    QCOMPARE(result, QStringLiteral("Ouvrir"));
}

void TestTsTranslator::testPluralSingular() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("test.ts");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(kValidTs);
    f.close();

    TsTranslator t;
    t.loadTsFile(path);
    QString result = t.translate("MainWindow", "%n mod(s) found", nullptr, 1);
    QCOMPARE(result, QStringLiteral("1 mod trouvé"));
}

void TestTsTranslator::testPluralPlural() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("test.ts");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(kValidTs);
    f.close();

    TsTranslator t;
    t.loadTsFile(path);
    QString result = t.translate("MainWindow", "%n mod(s) found", nullptr, 5);
    QCOMPARE(result, QStringLiteral("5 mods trouvés"));
}

void TestTsTranslator::testUnknownContextReturnsEmpty() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("test.ts");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(kValidTs);
    f.close();

    TsTranslator t;
    t.loadTsFile(path);
    QVERIFY(t.translate("NonExistent", "Settings").isEmpty());
}

void TestTsTranslator::testUnknownSourceReturnsEmpty() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.filePath("test.ts");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(kValidTs);
    f.close();

    TsTranslator t;
    t.loadTsFile(path);
    QVERIFY(t.translate("MainWindow", "NonExistent").isEmpty());
}

void TestTsTranslator::testEmptyAfterConstruction() {
    TsTranslator t;
    QVERIFY(t.isEmpty());
}

void TestTsTranslator::testInvalidFileReturnsFalse() {
    TsTranslator t;
    QVERIFY(!t.loadTsFile(QStringLiteral("/nonexistent/path/file.ts")));
    QVERIFY(t.isEmpty());
}

void TestTsTranslator::testLanguageDiscovery() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // Create a locales subdirectory with a .ts file
    QDir localesDir(dir.filePath("locales"));
    QVERIFY(localesDir.mkpath("."));

    QFile f(localesDir.filePath("TrenchKit_fr.ts"));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(kValidTs);
    f.close();

    // Verify the file was created
    QVERIFY(QFile::exists(localesDir.filePath("TrenchKit_fr.ts")));
}

QTEST_MAIN(TestTsTranslator)
#include "TestTsTranslator.moc"
