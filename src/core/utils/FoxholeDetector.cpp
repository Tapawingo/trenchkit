#include "FoxholeDetector.h"
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QString FoxholeDetector::detectInstallPath() {
    qDebug() << "Starting Foxhole installation detection...";

    // Check Steam library paths
    QStringList steamPaths = getSteamLibraryPaths();
    qDebug() << "Found" << steamPaths.size() << "Steam library paths";

    for (const QString &steamPath : steamPaths) {
        qDebug() << "Checking Steam library:" << steamPath;
        QString foxholePath = checkPath(steamPath + "/steamapps/common/Foxhole");
        if (!foxholePath.isEmpty()) {
            qDebug() << "Found Foxhole at:" << foxholePath;
            return foxholePath;
        }
    }

    // Check common installation paths
    QStringList commonPaths = {
        "C:/Program Files (x86)/Steam/steamapps/common/Foxhole",
        "C:/Program Files/Steam/steamapps/common/Foxhole",
        "D:/Steam/steamapps/common/Foxhole",
        "D:/SteamLibrary/steamapps/common/Foxhole",
        "E:/Steam/steamapps/common/Foxhole",
        "E:/SteamLibrary/steamapps/common/Foxhole",
        "F:/Steam/steamapps/common/Foxhole",
        "F:/SteamLibrary/steamapps/common/Foxhole"
    };

    qDebug() << "Checking common installation paths...";
    for (const QString &path : commonPaths) {
        QString foxholePath = checkPath(path);
        if (!foxholePath.isEmpty()) {
            qDebug() << "Found Foxhole at:" << foxholePath;
            return foxholePath;
        }
    }

    qDebug() << "Foxhole installation not found";
    return QString(); // Not found
}

bool FoxholeDetector::isValidInstallPath(const QString &path) {
    return !checkPath(path).isEmpty();
}

QStringList FoxholeDetector::getSteamLibraryPaths() {
    QStringList paths;

    // Try to read Steam config to find library folders
#ifdef Q_OS_WIN
    // Check Windows registry for Steam installation - try both 64-bit and 32-bit registry
    QSettings steamReg64("HKEY_LOCAL_MACHINE\\SOFTWARE\\Valve\\Steam", QSettings::NativeFormat);
    QSettings steamReg32("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Valve\\Steam", QSettings::NativeFormat);

    QString steamPath = steamReg64.value("InstallPath").toString();
    if (steamPath.isEmpty()) {
        steamPath = steamReg32.value("InstallPath").toString();
    }

    // Also try current user registry
    if (steamPath.isEmpty()) {
        QSettings steamRegUser("HKEY_CURRENT_USER\\Software\\Valve\\Steam", QSettings::NativeFormat);
        steamPath = steamRegUser.value("SteamPath").toString();
    }

    qDebug() << "Steam path from registry:" << steamPath;

    if (!steamPath.isEmpty()) {
        // Normalize path separators
        steamPath = QDir::fromNativeSeparators(steamPath);
        paths.append(steamPath);

        // Try to parse libraryfolders.vdf
        QString vdfPath = steamPath + "/steamapps/libraryfolders.vdf";
        qDebug() << "Looking for libraryfolders.vdf at:" << vdfPath;

        QFile vdfFile(vdfPath);
        if (vdfFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Parsing libraryfolders.vdf...";
            QTextStream in(&vdfFile);
            QString content = in.readAll();
            vdfFile.close();

            // Parse VDF format - look for "path" entries
            QStringList lines = content.split('\n');
            for (const QString &line : lines) {
                QString trimmed = line.trimmed();

                // Look for "path" key
                if (trimmed.startsWith("\"path\"")) {
                    // Extract value between quotes after the key
                    int firstQuote = trimmed.indexOf('\"', 6); // Skip "path"
                    int secondQuote = trimmed.indexOf('\"', firstQuote + 1);
                    int thirdQuote = trimmed.indexOf('\"', secondQuote + 1);
                    int fourthQuote = trimmed.indexOf('\"', thirdQuote + 1);

                    if (thirdQuote > 0 && fourthQuote > thirdQuote) {
                        QString libPath = trimmed.mid(thirdQuote + 1, fourthQuote - thirdQuote - 1);
                        // Convert double backslashes to forward slashes
                        libPath = libPath.replace("\\\\", "/");
                        libPath = QDir::fromNativeSeparators(libPath);

                        if (!libPath.isEmpty() && QDir(libPath).exists()) {
                            qDebug() << "Found library path:" << libPath;
                            paths.append(libPath);
                        }
                    }
                }
            }
        } else {
            qDebug() << "Could not open libraryfolders.vdf";
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

    qDebug() << "Total Steam library paths found:" << paths.size();
    return paths;
}

QString FoxholeDetector::checkPath(const QString &basePath) {
    if (basePath.isEmpty()) {
        return QString();
    }

    QDir dir(basePath);
    if (!dir.exists()) {
        qDebug() << "  Path does not exist:" << basePath;
        return QString();
    }

    qDebug() << "  Checking path:" << dir.absolutePath();

    // List of possible Foxhole executables to check
    QStringList possibleExes = {
        "Foxhole.exe",
        "FoxholeClient.exe",
        "FoxholeClient-Win64-Shipping.exe"
    };

    // Check for executable in root directory
    for (const QString &exe : possibleExes) {
        QString exePath = dir.filePath(exe);
        if (QFile::exists(exePath)) {
            qDebug() << "    Found executable:" << exe;
            return dir.absolutePath();
        }
    }

    // Check for Foxhole executable in War/Binaries/Win64/ subdirectory
    QDir warDir = dir;
    if (warDir.cd("War") && warDir.cd("Binaries") && warDir.cd("Win64")) {
        qDebug() << "    Checking War/Binaries/Win64/";
        for (const QString &exe : possibleExes) {
            QString exePath = warDir.filePath(exe);
            if (QFile::exists(exePath)) {
                qDebug() << "      Found executable:" << exe;
                return dir.absolutePath();
            }
        }
    }

    // Also check for other Foxhole-specific files/folders
    QStringList indicators = {
        "War/Content/Paks",
        "War/Content/Movies"
    };

    for (const QString &indicator : indicators) {
        if (QDir(dir.filePath(indicator)).exists()) {
            qDebug() << "    Found indicator:" << indicator;
            return dir.absolutePath();
        }
    }

    qDebug() << "    No Foxhole installation found at this path";
    return QString();
}
