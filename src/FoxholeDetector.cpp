#include "FoxholeDetector.h"
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

QString FoxholeDetector::detectInstallPath() {
    // Check Steam library paths
    QStringList steamPaths = getSteamLibraryPaths();
    for (const QString &steamPath : steamPaths) {
        QString foxholePath = checkPath(steamPath + "/steamapps/common/Foxhole");
        if (!foxholePath.isEmpty()) {
            return foxholePath;
        }
    }

    // Check common installation paths
    QStringList commonPaths = {
        "C:/Program Files (x86)/Steam/steamapps/common/Foxhole",
        "C:/Program Files/Steam/steamapps/common/Foxhole",
        "D:/SteamLibrary/steamapps/common/Foxhole",
        "E:/SteamLibrary/steamapps/common/Foxhole"
    };

    for (const QString &path : commonPaths) {
        QString foxholePath = checkPath(path);
        if (!foxholePath.isEmpty()) {
            return foxholePath;
        }
    }

    return QString(); // Not found
}

bool FoxholeDetector::isValidInstallPath(const QString &path) {
    return !checkPath(path).isEmpty();
}

QStringList FoxholeDetector::getSteamLibraryPaths() {
    QStringList paths;

    // Try to read Steam config to find library folders
#ifdef Q_OS_WIN
    // Check Windows registry for Steam installation
    QSettings steamReg("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Valve\\Steam", QSettings::NativeFormat);
    QString steamPath = steamReg.value("InstallPath").toString();

    if (!steamPath.isEmpty()) {
        paths.append(steamPath);

        // Try to parse libraryfolders.vdf
        QString vdfPath = steamPath + "/steamapps/libraryfolders.vdf";
        QFile vdfFile(vdfPath);
        if (vdfFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&vdfFile);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                // Simple parsing: look for "path" entries
                if (line.contains("\"path\"")) {
                    int start = line.indexOf("\"", 10) + 1;
                    int end = line.lastIndexOf("\"");
                    if (start > 0 && end > start) {
                        QString libPath = line.mid(start, end - start);
                        // Convert double backslashes to forward slashes
                        libPath = libPath.replace("\\\\", "/");
                        paths.append(libPath);
                    }
                }
            }
            vdfFile.close();
        }
    }
#elif defined(Q_OS_LINUX)
    QString steamPath = QDir::homePath() + "/.steam/steam";
    if (QDir(steamPath).exists()) {
        paths.append(steamPath);
    }
#elif defined(Q_OS_MAC)
    QString steamPath = QDir::homePath() + "/Library/Application Support/Steam";
    if (QDir(steamPath).exists()) {
        paths.append(steamPath);
    }
#endif

    return paths;
}

QString FoxholeDetector::checkPath(const QString &basePath) {
    if (basePath.isEmpty()) {
        return QString();
    }

    QDir dir(basePath);
    if (!dir.exists()) {
        return QString();
    }

    // Check for Foxhole.exe in root
    if (QFile::exists(dir.filePath("Foxhole.exe"))) {
        return dir.absolutePath();
    }

    // Check for Foxhole client executable in War/Binaries/Win64/
    QDir warDir = dir;
    if (warDir.cd("War") && warDir.cd("Binaries") && warDir.cd("Win64")) {
        if (QFile::exists(warDir.filePath("Foxhole.exe")) ||
            QFile::exists(warDir.filePath("FoxholeClient-Win64-Shipping.exe"))) {
            return dir.absolutePath();
        }
    }

    return QString();
}
