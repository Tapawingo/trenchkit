#include "UpdateArchiveExtractor.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <zip.h>

bool UpdateArchiveExtractor::extractZip(const QString &zipPath,
                                        const QString &destDir,
                                        QString *error) {
    QFileInfo zipInfo(zipPath);
    if (!zipInfo.exists() || !zipInfo.isFile()) {
        if (error) {
            *error = "Archive file does not exist.";
        }
        return false;
    }

    zip_t *zip = zip_open(zipPath.toUtf8().constData(), 0, 'r');
    if (!zip) {
        if (error) {
            *error = "Failed to open archive.";
        }
        return false;
    }

    QDir dir;
    if (!dir.mkpath(destDir)) {
        zip_close(zip);
        if (error) {
            *error = "Failed to create destination directory.";
        }
        return false;
    }

    const int totalEntries = zip_entries_total(zip);
    for (int i = 0; i < totalEntries; ++i) {
        if (zip_entry_openbyindex(zip, i) < 0) {
            continue;
        }

        const char *entryName = zip_entry_name(zip);
        if (!entryName) {
            zip_entry_close(zip);
            continue;
        }

        const QString entryPath = QString::fromUtf8(entryName);
        const QString outputPath = QDir(destDir).filePath(entryPath);

        if (zip_entry_isdir(zip)) {
            dir.mkpath(outputPath);
            zip_entry_close(zip);
            continue;
        }

        QFileInfo outInfo(outputPath);
        if (!dir.mkpath(outInfo.path())) {
            zip_entry_close(zip);
            zip_close(zip);
            if (error) {
                *error = "Failed to create output directory.";
            }
            return false;
        }

        void *buf = nullptr;
        size_t bufsize = 0;
        const int readResult = zip_entry_read(zip, &buf, &bufsize);
        if (readResult != 0 || !buf) {
            zip_entry_close(zip);
            zip_close(zip);
            if (error) {
                *error = "Failed to read archive entry.";
            }
            return false;
        }

        QFile outFile(outputPath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            free(buf);
            zip_entry_close(zip);
            zip_close(zip);
            if (error) {
                *error = "Failed to write extracted file.";
            }
            return false;
        }

        outFile.write(static_cast<const char *>(buf), static_cast<qint64>(bufsize));
        outFile.close();
        free(buf);

        zip_entry_close(zip);
    }

    zip_close(zip);
    return true;
}
