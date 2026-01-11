#include "ArchiveExtractor.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUuid>
#include <QDebug>
#include <zip.h>
#include <archive.h>
#include <archive_entry.h>

ArchiveExtractor::ArchiveExtractor(QObject *parent)
    : QObject(parent)
{
}

ArchiveExtractor::ArchiveFormat ArchiveExtractor::detectFormat(const QString &filePath) const {
    QString lower = filePath.toLower();
    qDebug() << "ArchiveExtractor: Detecting format for:" << filePath;

    if (lower.endsWith(".tar.gz") || lower.endsWith(".tgz")) {
        qDebug() << "ArchiveExtractor: Detected format: TarGz";
        return ArchiveFormat::TarGz;
    }
    if (lower.endsWith(".tar.bz2") || lower.endsWith(".tbz2")) {
        qDebug() << "ArchiveExtractor: Detected format: TarBz2";
        return ArchiveFormat::TarBz2;
    }
    if (lower.endsWith(".tar.xz") || lower.endsWith(".txz")) {
        qDebug() << "ArchiveExtractor: Detected format: TarXz";
        return ArchiveFormat::TarXz;
    }
    if (lower.endsWith(".zip")) {
        qDebug() << "ArchiveExtractor: Detected format: Zip";
        return ArchiveFormat::Zip;
    }
    if (lower.endsWith(".rar")) {
        qDebug() << "ArchiveExtractor: Detected format: Rar";
        return ArchiveFormat::Rar;
    }
    if (lower.endsWith(".7z")) {
        qDebug() << "ArchiveExtractor: Detected format: 7z";
        return ArchiveFormat::SevenZip;
    }

    qDebug() << "ArchiveExtractor: Unknown format";
    return ArchiveFormat::Unknown;
}

ArchiveExtractor::ExtractResult ArchiveExtractor::extractWithLibarchive(
    const QString &archivePath) {

    qDebug() << "ArchiveExtractor: extractWithLibarchive called for:" << archivePath;

    QFileInfo archiveInfo(archivePath);
    if (!archiveInfo.exists() || !archiveInfo.isFile()) {
        qDebug() << "ArchiveExtractor: Archive file does not exist or is not a file";
        return {false, {}, "", "Archive file does not exist"};
    }

    qDebug() << "ArchiveExtractor: Archive exists, size:" << archiveInfo.size() << "bytes";

    struct archive *a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    qDebug() << "ArchiveExtractor: Opening archive with libarchive...";
    int r = archive_read_open_filename(a, archivePath.toUtf8().constData(), 10240);
    if (r != ARCHIVE_OK) {
        QString error = QString("Failed to open archive: %1")
                           .arg(archive_error_string(a));
        qDebug() << "ArchiveExtractor: ERROR -" << error;
        archive_read_free(a);
        emit errorOccurred(error);
        return {false, {}, "", error};
    }

    qDebug() << "ArchiveExtractor: Archive opened successfully";

    QString tempDir = createTempDir();
    qDebug() << "ArchiveExtractor: Temp directory:" << tempDir;

    QDir dir;
    if (!dir.mkpath(tempDir)) {
        qDebug() << "ArchiveExtractor: ERROR - Failed to create temp directory";
        archive_read_free(a);
        return {false, {}, "", "Failed to create temporary directory"};
    }

    QStringList pakFiles;
    struct archive_entry *entry;
    int entryCount = 0;

    qDebug() << "ArchiveExtractor: Reading archive entries...";
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *entryName = archive_entry_pathname(entry);
        if (!entryName) {
            qDebug() << "ArchiveExtractor: Entry with null name, skipping";
            continue;
        }

        QString fileName = QString::fromUtf8(entryName);
        entryCount++;
        qDebug() << "ArchiveExtractor: Entry" << entryCount << ":" << fileName;

        if (isPakFile(fileName)) {
            qDebug() << "ArchiveExtractor: Found .pak file:" << fileName;
            QString baseName = QFileInfo(fileName).fileName();
            QString destPath = tempDir + "/" + baseName;

            const void *buff;
            size_t size;
            int64_t offset;

            QFile outFile(destPath);
            if (!outFile.open(QIODevice::WriteOnly)) {
                qDebug() << "ArchiveExtractor: ERROR - Failed to open output file:" << destPath;
                continue;
            }

            qDebug() << "ArchiveExtractor: Extracting to:" << destPath;
            qint64 totalBytes = 0;
            while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
                outFile.write(static_cast<const char*>(buff), size);
                totalBytes += size;
            }

            outFile.close();
            qDebug() << "ArchiveExtractor: Extracted" << totalBytes << "bytes";
            pakFiles.append(destPath);
        } else {
            archive_read_data_skip(a);
        }
    }

    archive_read_free(a);

    qDebug() << "ArchiveExtractor: Total entries processed:" << entryCount;
    qDebug() << "ArchiveExtractor: .pak files found:" << pakFiles.size();

    if (pakFiles.isEmpty()) {
        qDebug() << "ArchiveExtractor: ERROR - No .pak files found in archive";
        cleanupTempDir(tempDir);
        return {false, {}, "", "No .pak files found in archive"};
    }

    qDebug() << "ArchiveExtractor: Extraction successful";
    return {true, pakFiles, tempDir, ""};
}

ArchiveExtractor::ExtractResult ArchiveExtractor::extractPakFiles(
    const QString &archivePath) {

    qDebug() << "ArchiveExtractor: extractPakFiles called for:" << archivePath;

    ArchiveFormat format = detectFormat(archivePath);

    if (format == ArchiveFormat::Zip) {
        qDebug() << "ArchiveExtractor: Using libarchive for .zip with fallback to zip library";
        ExtractResult result = extractWithLibarchive(archivePath);
        if (!result.success) {
            qDebug() << "ArchiveExtractor: libarchive failed, falling back to zip library";
            return extractWithZip(archivePath);
        }
        return result;
    } else if (format == ArchiveFormat::Unknown) {
        qDebug() << "ArchiveExtractor: ERROR - Unknown or unsupported archive format";
        return {false, {}, "", "Unknown or unsupported archive format"};
    } else {
        qDebug() << "ArchiveExtractor: Using libarchive for extraction";
        return extractWithLibarchive(archivePath);
    }
}

ArchiveExtractor::ExtractResult ArchiveExtractor::extractWithZip(const QString &zipPath) {
    QFileInfo zipInfo(zipPath);
    if (!zipInfo.exists() || !zipInfo.isFile()) {
        return {false, {}, "", "Archive file does not exist"};
    }

    zip_t *zip = zip_open(zipPath.toUtf8().constData(), 0, 'r');
    if (!zip) {
        QString error = "Failed to open archive";
        emit errorOccurred(error);
        return {false, {}, "", error};
    }

    QString tempDir = createTempDir();
    QDir dir;
    if (!dir.mkpath(tempDir)) {
        zip_close(zip);
        return {false, {}, "", "Failed to create temporary directory"};
    }

    QStringList pakFiles;
    int totalEntries = zip_entries_total(zip);

    for (int i = 0; i < totalEntries; ++i) {
        if (zip_entry_openbyindex(zip, i) < 0) {
            continue;
        }

        const char *entryName = zip_entry_name(zip);
        QString fileName = QString::fromUtf8(entryName);

        if (isPakFile(fileName)) {
            QString baseName = QFileInfo(fileName).fileName();
            QString destPath = tempDir + "/" + baseName;

            void *buf = nullptr;
            size_t bufsize = 0;

            const ssize_t readResult = zip_entry_read(zip, &buf, &bufsize);
            if (readResult >= 0) {
                QFile outFile(destPath);
                if (outFile.open(QIODevice::WriteOnly)) {
                    if (buf && bufsize > 0) {
                        outFile.write(static_cast<const char*>(buf),
                                      static_cast<qint64>(bufsize));
                    }
                    outFile.close();
                    pakFiles.append(destPath);
                }
            }
            if (buf) {
                free(buf);
            }
        }

        zip_entry_close(zip);
    }

    zip_close(zip);

    if (pakFiles.isEmpty()) {
        cleanupTempDir(tempDir);
        return {false, {}, "", "No .pak files found in archive"};
    }

    return {true, pakFiles, tempDir, ""};
}

void ArchiveExtractor::cleanupTempDir(const QString &tempDir) {
    QDir dir(tempDir);
    if (dir.exists()) {
        dir.removeRecursively();
    }
}

bool ArchiveExtractor::isPakFile(const QString &fileName) const {
    return fileName.endsWith(".pak", Qt::CaseInsensitive);
}

QString ArchiveExtractor::createTempDir() const {
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return tempPath + "/TrenchKit_extract_" + uuid;
}
