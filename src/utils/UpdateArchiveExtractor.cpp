#include "UpdateArchiveExtractor.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <zip.h>
#include <archive.h>
#include <archive_entry.h>

bool UpdateArchiveExtractor::extractArchive(const QString &archivePath,
                                           const QString &destDir,
                                           QString *error) {
    QString lower = archivePath.toLower();

    if (lower.endsWith(".zip")) {
        if (!extractWithLibarchive(archivePath, destDir, error)) {
            return extractWithZip(archivePath, destDir, error);
        }
        return true;
    }

    return extractWithLibarchive(archivePath, destDir, error);
}

bool UpdateArchiveExtractor::extractWithLibarchive(const QString &archivePath,
                                                   const QString &destDir,
                                                   QString *error) {
    QFileInfo archiveInfo(archivePath);
    if (!archiveInfo.exists() || !archiveInfo.isFile()) {
        if (error) {
            *error = "Archive file does not exist.";
        }
        return false;
    }

    struct archive *a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    int r = archive_read_open_filename(a, archivePath.toUtf8().constData(), 10240);
    if (r != ARCHIVE_OK) {
        if (error) {
            *error = QString("Failed to open archive: %1").arg(archive_error_string(a));
        }
        archive_read_free(a);
        return false;
    }

    QDir dir;
    if (!dir.mkpath(destDir)) {
        if (error) {
            *error = "Failed to create destination directory.";
        }
        archive_read_free(a);
        return false;
    }

    struct archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *entryName = archive_entry_pathname(entry);
        if (!entryName) {
            continue;
        }

        const QString entryPath = QString::fromUtf8(entryName);
        const QString outputPath = QDir(destDir).filePath(entryPath);

        if (archive_entry_filetype(entry) == AE_IFDIR) {
            dir.mkpath(outputPath);
            continue;
        }

        QFileInfo outInfo(outputPath);
        if (!dir.mkpath(outInfo.path())) {
            if (error) {
                *error = "Failed to create output directory.";
            }
            archive_read_free(a);
            return false;
        }

        QFile outFile(outputPath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            if (error) {
                *error = "Failed to write extracted file.";
            }
            archive_read_free(a);
            return false;
        }

        const void *buff;
        size_t size;
        int64_t offset;

        while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
            outFile.write(static_cast<const char*>(buff), size);
        }

        outFile.close();
    }

    archive_read_free(a);
    return true;
}

bool UpdateArchiveExtractor::extractWithZip(const QString &zipPath,
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
        const ssize_t readResult = zip_entry_read(zip, &buf, &bufsize);
        if (readResult < 0) {
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

        if (buf && bufsize > 0) {
            outFile.write(static_cast<const char *>(buf),
                          static_cast<qint64>(bufsize));
        }
        outFile.close();
        if (buf) {
            free(buf);
        }

        zip_entry_close(zip);
    }

    zip_close(zip);
    return true;
}
