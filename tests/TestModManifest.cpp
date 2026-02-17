#include <QtTest/QtTest>

#include "core/utils/PakFileReader.h"
#include "core/utils/ModManifestReader.h"

#include <QDataStream>
#include <QTemporaryDir>
#include <QtEndian>

static const QByteArray kTestManifestXml =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<ModManifest version=\"1\">\r\n"
    "  <Id>test.mod.example</Id>\r\n"
    "  <Name>Test Mod</Name>\r\n"
    "  <Version>1.2.3</Version>\r\n"
    "  <Author>TestAuthor</Author>\r\n"
    "  <Description>A test mod for unit testing.</Description>\r\n"
    "  <Notice icon=\"warning\">This is a test notice.</Notice>\r\n"
    "  <Dependencies>\r\n"
    "    <Dependency id=\"test.mod.dep\" minVersion=\"1.0.0\" required=\"true\" />\r\n"
    "  </Dependencies>\r\n"
    "  <Tags>\r\n"
    "    <Tag>Testing</Tag>\r\n"
    "  </Tags>\r\n"
    "</ModManifest>\r\n";

namespace {

void writeString(QDataStream &s, const QString &str) {
    QByteArray utf8 = str.toUtf8();
    utf8.append('\0');
    s << static_cast<quint32>(utf8.size());
    s.writeRawData(utf8.constData(), utf8.size());
}

void writeRecordHeader(QDataStream &s, quint64 fileSize) {
    s << quint64(0);       // offset
    s << fileSize;         // compressed size
    s << fileSize;         // uncompressed size
    s << quint32(0);       // compression method
    for (int i = 0; i < 20; ++i)
        s << quint8(0);    // SHA1 hash
    s << quint8(0);        // encrypted
    s << quint32(0);       // block size
}

static constexpr quint64 kV3RecordHeaderSize = 8 + 8 + 8 + 4 + 20 + 1 + 4; // 53

void writeFooter(QDataStream &s, quint64 indexOffset, quint64 indexSize) {
    s << quint32(0x5A6F12E1); // magic
    s << quint32(3);          // version
    s << indexOffset;
    s << indexSize;
    for (int i = 0; i < 20; ++i)
        s << quint8(0);       // index hash
}

void writeIndexEntry(QDataStream &s, const QString &filePath, quint64 offset,
                     quint64 compressedSize, quint64 uncompressedSize,
                     quint32 compressionMethod,
                     const QVector<QPair<quint64, quint64>> &blocks = {},
                     quint32 blockSize = 0) {
    writeString(s, filePath);
    s << offset;
    s << compressedSize;
    s << uncompressedSize;
    s << compressionMethod;
    for (int i = 0; i < 20; ++i)
        s << quint8(0); // hash
    if (compressionMethod != 0) {
        s << static_cast<quint32>(blocks.size());
        for (const auto &[start, end] : blocks) {
            s << start;
            s << end;
        }
    }
    s << quint8(0);    // encrypted
    s << blockSize;    // block size
}

QByteArray buildUncompressedPak(const QString &filePath, const QByteArray &fileData) {
    QByteArray pakData;
    QDataStream s(&pakData, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);

    quint64 fileSize = fileData.size();

    // Data section: embedded record header + raw file data
    writeRecordHeader(s, fileSize);
    s.writeRawData(fileData.constData(), fileData.size());

    quint64 indexOffset = pakData.size();

    // Index: mount point + record count + entry
    writeString(s, QStringLiteral("../../../"));
    s << quint32(1);
    writeIndexEntry(s, filePath, 0, fileSize, fileSize, 0);

    quint64 indexSize = pakData.size() - indexOffset;
    writeFooter(s, indexOffset, indexSize);

    return pakData;
}

QByteArray buildCompressedPak(const QString &filePath, const QByteArray &fileData) {
    QByteArray pakData;
    QDataStream s(&pakData, QIODevice::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);

    quint64 fileSize = fileData.size();

    // Compress file data: qCompress produces [4-byte BE size][zlib stream]
    QByteArray zlibStream = qCompress(fileData).mid(4);
    quint64 compressedSize = zlibStream.size();

    // Data section: embedded record header (uncompressed) + compressed block
    writeRecordHeader(s, fileSize);

    quint64 blockStart = pakData.size(); // = kV3RecordHeaderSize
    s.writeRawData(zlibStream.constData(), zlibStream.size());
    quint64 blockEnd = pakData.size();

    quint64 indexOffset = pakData.size();

    // Index
    writeString(s, QStringLiteral("../../../"));
    s << quint32(1);
    QVector<QPair<quint64, quint64>> blocks = {{blockStart, blockEnd}};
    writeIndexEntry(s, filePath, 0, compressedSize, fileSize, 1, blocks, 65536);

    quint64 indexSize = pakData.size() - indexOffset;
    writeFooter(s, indexOffset, indexSize);

    return pakData;
}

} // anonymous namespace

class TestModManifest : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;

    QString writeTempPak(const QByteArray &pakData, const QString &name = QStringLiteral("test.pak")) {
        QString path = m_tempDir.filePath(name);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            return {};
        }
        file.write(pakData);
        return path;
    }

private slots:
    void extractFilePaths_uncompressed();
    void extractFilePaths_compressed();
    void extractFile_uncompressed();
    void extractFile_compressed();
    void extractFile_notFound();
    void readManifest_uncompressed();
    void readManifest_compressed();
    void readManifest_parsesAllFields();
};

void TestModManifest::extractFilePaths_uncompressed() {
    QByteArray pak = buildUncompressedPak(QStringLiteral("War/Mod/Manifest.xml"), kTestManifestXml);
    QString path = writeTempPak(pak, QTest::currentTestFunction() + QStringLiteral(".pak"));

    auto result = PakFileReader::extractFilePaths(path);
    QVERIFY2(result.success, qPrintable(result.error));
    QCOMPARE(result.filePaths.size(), 1);
    QCOMPARE(result.filePaths.first(), QStringLiteral("War/Mod/Manifest.xml"));
}

void TestModManifest::extractFilePaths_compressed() {
    QByteArray pak = buildCompressedPak(QStringLiteral("War/Mod/Manifest.xml"), kTestManifestXml);
    QString path = writeTempPak(pak, QTest::currentTestFunction() + QStringLiteral(".pak"));

    auto result = PakFileReader::extractFilePaths(path);
    QVERIFY2(result.success, qPrintable(result.error));
    QCOMPARE(result.filePaths.size(), 1);
    QCOMPARE(result.filePaths.first(), QStringLiteral("War/Mod/Manifest.xml"));
}

void TestModManifest::extractFile_uncompressed() {
    QByteArray pak = buildUncompressedPak(QStringLiteral("War/Mod/Manifest.xml"), kTestManifestXml);
    QString path = writeTempPak(pak, QTest::currentTestFunction() + QStringLiteral(".pak"));

    QByteArray data;
    QString error;
    bool ok = PakFileReader::extractFile(path, {QStringLiteral("Mod/Manifest.xml")}, &data, &error);
    QVERIFY2(ok, qPrintable(error));
    QCOMPARE(data, kTestManifestXml);
}

void TestModManifest::extractFile_compressed() {
    QByteArray pak = buildCompressedPak(QStringLiteral("War/Mod/Manifest.xml"), kTestManifestXml);
    QString path = writeTempPak(pak, QTest::currentTestFunction() + QStringLiteral(".pak"));

    QByteArray data;
    QString error;
    bool ok = PakFileReader::extractFile(path, {QStringLiteral("Mod/Manifest.xml")}, &data, &error);
    QVERIFY2(ok, qPrintable(error));
    QCOMPARE(data, kTestManifestXml);
}

void TestModManifest::extractFile_notFound() {
    QByteArray pak = buildUncompressedPak(QStringLiteral("War/SomeOtherFile.uasset"), "dummy");
    QString path = writeTempPak(pak, QTest::currentTestFunction() + QStringLiteral(".pak"));

    QByteArray data;
    QString error;
    bool ok = PakFileReader::extractFile(path, {QStringLiteral("Mod/Manifest.xml")}, &data, &error);
    QVERIFY(!ok);
    QVERIFY(error.contains(QStringLiteral("not found")));
}

void TestModManifest::readManifest_uncompressed() {
    QByteArray pak = buildUncompressedPak(QStringLiteral("War/Mod/Manifest.xml"), kTestManifestXml);
    QString path = writeTempPak(pak, QTest::currentTestFunction() + QStringLiteral(".pak"));

    ModManifest manifest;
    QString error;
    bool ok = ModManifestReader::readFromPak(path, &manifest, &error);
    QVERIFY2(ok, qPrintable(error));
    QCOMPARE(manifest.name, QStringLiteral("Test Mod"));
}

void TestModManifest::readManifest_compressed() {
    QByteArray pak = buildCompressedPak(QStringLiteral("War/Mod/Manifest.xml"), kTestManifestXml);
    QString path = writeTempPak(pak, QTest::currentTestFunction() + QStringLiteral(".pak"));

    ModManifest manifest;
    QString error;
    bool ok = ModManifestReader::readFromPak(path, &manifest, &error);
    QVERIFY2(ok, qPrintable(error));
    QCOMPARE(manifest.name, QStringLiteral("Test Mod"));
}

void TestModManifest::readManifest_parsesAllFields() {
    QByteArray pak = buildUncompressedPak(QStringLiteral("War/Mod/Manifest.xml"), kTestManifestXml);
    QString path = writeTempPak(pak, QTest::currentTestFunction() + QStringLiteral(".pak"));

    ModManifest manifest;
    QString error;
    bool ok = ModManifestReader::readFromPak(path, &manifest, &error);
    QVERIFY2(ok, qPrintable(error));

    QCOMPARE(manifest.id, QStringLiteral("test.mod.example"));
    QCOMPARE(manifest.name, QStringLiteral("Test Mod"));
    QCOMPARE(manifest.version, QStringLiteral("1.2.3"));
    QCOMPARE(manifest.authors.size(), 1);
    QCOMPARE(manifest.authors.first(), QStringLiteral("TestAuthor"));
    QCOMPARE(manifest.description, QStringLiteral("A test mod for unit testing."));
    QCOMPARE(manifest.noticeIcon, QStringLiteral("warning"));
    QCOMPARE(manifest.noticeText, QStringLiteral("This is a test notice."));
    QCOMPARE(manifest.dependencies.size(), 1);
    QCOMPARE(manifest.dependencies.first().id, QStringLiteral("test.mod.dep"));
    QCOMPARE(manifest.dependencies.first().minVersion, QStringLiteral("1.0.0"));
    QVERIFY(manifest.dependencies.first().required);
    QCOMPARE(manifest.tags.size(), 1);
    QCOMPARE(manifest.tags.first(), QStringLiteral("Testing"));
}

QTEST_MAIN(TestModManifest)
#include "TestModManifest.moc"
