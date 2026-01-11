#include "UpdateCleanup.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QSaveFile>
#include <QDebug>
#include <QStandardPaths>
#include <algorithm>

static QString currentVersionString() {
#ifdef TRENCHKIT_VERSION
    return QStringLiteral(TRENCHKIT_VERSION);
#else
    return QStringLiteral("0.0.0");
#endif
}

static void cleanupStagingDirectories(const QDir &updatesDir) {
    QDir stagingDir(updatesDir.filePath("staging"));
    if (!stagingDir.exists()) {
        return;
    }

    QFileInfoList entries = stagingDir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot);
    std::sort(entries.begin(), entries.end(), [](const QFileInfo &a, const QFileInfo &b) {
        return a.lastModified() > b.lastModified();
    });

    for (int i = 2; i < entries.size(); ++i) {
        QDir oldDir(entries[i].absoluteFilePath());
        if (!oldDir.removeRecursively()) {
            qWarning() << "Update cleanup: failed to remove staging dir"
                       << entries[i].absoluteFilePath();
        }
    }
}

static void cleanupArchives(const QDir &updatesDir) {
    const QStringList filters = {
        "*.zip",
        "*.pak",
        "*.7z",
        "*.rar",
        "*.tar",
        "*.tar.gz",
        "*.tar.bz2",
        "*.tar.xz"
    };

    QFileInfoList entries = updatesDir.entryInfoList(
        filters, QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &entry : entries) {
        if (!QFile::remove(entry.absoluteFilePath())) {
            qWarning() << "Update cleanup: failed to remove"
                       << entry.absoluteFilePath();
        }
    }
}

static QString readMarkerVersion(const QString &markerPath) {
    QFile marker(markerPath);
    if (!marker.exists() || !marker.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return QString::fromUtf8(marker.readAll()).trimmed();
}

static void writeMarkerVersion(const QString &markerPath, const QString &version) {
    QSaveFile marker(markerPath);
    if (!marker.open(QIODevice::WriteOnly)) {
        qWarning() << "Update cleanup: failed to write marker file";
        return;
    }
    marker.write(version.toUtf8());
    marker.commit();
}

void UpdateCleanup::run() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const QString updatesDirPath = QDir(base).filePath("updates");
    QDir updatesDir(updatesDirPath);
    if (!updatesDir.exists()) {
        updatesDir.mkpath(".");
    }

    const QString markerPath = updatesDir.filePath("last_installed_version.txt");
    const QString previousVersion = readMarkerVersion(markerPath);
    const QString currentVersion = currentVersionString();
    const bool versionChanged = !previousVersion.isEmpty() && previousVersion != currentVersion;

    if (versionChanged) {
        qInfo() << "Update cleanup: installed version" << currentVersion;
        cleanupArchives(updatesDir);
    }

    cleanupStagingDirectories(updatesDir);
    writeMarkerVersion(markerPath, currentVersion);
}

void UpdateCleanup::run(const QString &appDir) {
    QDir updatesDir(QDir(appDir).filePath("updates"));
    if (!updatesDir.exists()) {
        updatesDir.mkpath(".");
    }

    const QString markerPath = updatesDir.filePath("last_installed_version.txt");
    const QString previousVersion = readMarkerVersion(markerPath);
    const QString currentVersion = currentVersionString();
    const bool versionChanged = !previousVersion.isEmpty() && previousVersion != currentVersion;

    if (versionChanged) {
        qInfo() << "Update cleanup: installed version" << currentVersion;
        cleanupArchives(updatesDir);
    }

    cleanupStagingDirectories(updatesDir);
    writeMarkerVersion(markerPath, currentVersion);
}
