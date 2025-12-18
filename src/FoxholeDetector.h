#ifndef FOXHOLEDETECTOR_H
#define FOXHOLEDETECTOR_H

#include <QString>
#include <QStringList>

class FoxholeDetector {
public:
    // Try to automatically detect Foxhole installation
    static QString detectInstallPath();

    // Validate if a path contains a Foxhole installation
    static bool isValidInstallPath(const QString &path);

private:
    // Check common Steam library locations
    static QStringList getSteamLibraryPaths();

    // Check a specific path for Foxhole
    static QString checkPath(const QString &basePath);

    // Common Foxhole installation subpaths
    static constexpr const char* STEAM_APP_PATH = "steamapps/common/Foxhole";
    static constexpr int FOXHOLE_APP_ID = 505460;
};

#endif // FOXHOLEDETECTOR_H
