#include <QtTest/QtTest>

#include "core/utils/PakFileReader.h"
#include "core/utils/ModManifestReader.h"

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR ""
#endif

class TestModManifest : public QObject {
    Q_OBJECT

private slots:
    void extractFilePaths_falchion();
    void extractFilePaths_vehicleShared();
    void extractRawData_startsWithXml();
    void readManifest_falchion();
    void readManifest_vehicleShared();
};

static QString pakPath(const char *name) {
    return QStringLiteral(TEST_DATA_DIR) + QStringLiteral("/") + QString::fromLatin1(name);
}

void TestModManifest::extractFilePaths_falchion() {
    const QString path = pakPath("War-WindowsNoEditor_Vehicle_Col_AsT_Falchion_WG_Ver.2.0_MANIFESTED.pak");
    if (!QFile::exists(path)) {
        QSKIP("Test pak file not found");
    }

    auto result = PakFileReader::extractFilePaths(path);
    QVERIFY2(result.success, qPrintable(result.error));
    QVERIFY(!result.filePaths.isEmpty());

    bool hasManifest = false;
    for (const QString &fp : result.filePaths) {
        if (fp.toLower().contains(QStringLiteral("manifest.xml"))) {
            hasManifest = true;
            break;
        }
    }
    QVERIFY2(hasManifest, "Pak should contain Manifest.xml");
}

void TestModManifest::extractFilePaths_vehicleShared() {
    const QString path = pakPath("War-WindowsNoEditor_Vehicle_Shared_MANIFESTED.pak");
    if (!QFile::exists(path)) {
        QSKIP("Test pak file not found");
    }

    auto result = PakFileReader::extractFilePaths(path);
    QVERIFY2(result.success, qPrintable(result.error));
    QVERIFY(!result.filePaths.isEmpty());

    bool hasManifest = false;
    for (const QString &fp : result.filePaths) {
        if (fp.toLower().contains(QStringLiteral("manifest.xml"))) {
            hasManifest = true;
            break;
        }
    }
    QVERIFY2(hasManifest, "Pak should contain Manifest.xml");
}

void TestModManifest::extractRawData_startsWithXml() {
    const QString path = pakPath("War-WindowsNoEditor_Vehicle_Col_AsT_Falchion_WG_Ver.2.0_MANIFESTED.pak");
    if (!QFile::exists(path)) {
        QSKIP("Test pak file not found");
    }

    QByteArray data;
    QStringList candidates;
    candidates << QStringLiteral("Mod/Manifest.xml");
    QString error;
    bool ok = PakFileReader::extractFile(path, candidates, &data, &error);
    QVERIFY2(ok, qPrintable(QStringLiteral("extractFile failed: ") + error));
    QVERIFY(data.size() > 0);
    QVERIFY2(data.startsWith('<'), "Extracted data should start with XML");
    QVERIFY2(data.contains("</ModManifest>"), "Extracted data should contain closing tag");
}

void TestModManifest::readManifest_falchion() {
    const QString path = pakPath("War-WindowsNoEditor_Vehicle_Col_AsT_Falchion_WG_Ver.2.0_MANIFESTED.pak");
    if (!QFile::exists(path)) {
        QSKIP("Test pak file not found");
    }

    ModManifest manifest;
    QString error;
    bool ok = ModManifestReader::readFromPak(path, &manifest, &error);
    QVERIFY2(ok, qPrintable(QStringLiteral("readFromPak failed: ") + error));

    QCOMPARE(manifest.id, QStringLiteral("wolfgangIX.vehiclesOverhaulFalchion"));
    QCOMPARE(manifest.name, QStringLiteral("Vehicles Overhauled Falchion"));
    QCOMPARE(manifest.version, QStringLiteral("2.2.0"));
    QVERIFY(!manifest.authors.isEmpty());
    QCOMPARE(manifest.authors.first(), QStringLiteral("WolfgangIX"));
    QVERIFY(!manifest.description.isEmpty());
    QVERIFY(!manifest.noticeText.isEmpty());
    QCOMPARE(manifest.dependencies.size(), 1);
    QCOMPARE(manifest.dependencies.first().id, QStringLiteral("wolfgangIX.vehiclesOverhaulShared"));
}

void TestModManifest::readManifest_vehicleShared() {
    const QString path = pakPath("War-WindowsNoEditor_Vehicle_Shared_MANIFESTED.pak");
    if (!QFile::exists(path)) {
        QSKIP("Test pak file not found");
    }

    ModManifest manifest;
    QString error;
    bool ok = ModManifestReader::readFromPak(path, &manifest, &error);
    QVERIFY2(ok, qPrintable(QStringLiteral("readFromPak failed: ") + error));

    QCOMPARE(manifest.id, QStringLiteral("wolfgangIX.vehiclesOverhaulShared"));
    QCOMPARE(manifest.name, QStringLiteral("Vehicles Overhauled Shared"));
    QCOMPARE(manifest.version, QStringLiteral("2.2.0"));
    QVERIFY(!manifest.authors.isEmpty());
    QVERIFY(!manifest.description.isEmpty());
    QVERIFY(!manifest.noticeText.isEmpty());
    QCOMPARE(manifest.dependencies.size(), 0);
}

QTEST_MAIN(TestModManifest)
#include "TestModManifest.moc"
