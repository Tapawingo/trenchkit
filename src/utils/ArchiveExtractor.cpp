#include "ArchiveExtractor.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUuid>
#include <zip.h>

ArchiveExtractor::ArchiveExtractor(QObject *parent)
    : QObject(parent)
{
}

ArchiveExtractor::ExtractResult ArchiveExtractor::extractPakFiles(const QString &zipPath) {
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

            if (zip_entry_read(zip, &buf, &bufsize) == 0) {
                QFile outFile(destPath);
                if (outFile.open(QIODevice::WriteOnly)) {
                    outFile.write(static_cast<const char*>(buf), bufsize);
                    outFile.close();
                    pakFiles.append(destPath);
                }
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
