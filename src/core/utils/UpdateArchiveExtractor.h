#pragma once

#include <QString>

class UpdateArchiveExtractor {
public:
    static bool extractArchive(const QString &archivePath, const QString &destDir, QString *error);

private:
    static bool extractWithLibarchive(const QString &archivePath,
                                     const QString &destDir,
                                     QString *error);
    static bool extractWithZip(const QString &archivePath,
                              const QString &destDir,
                              QString *error);
};
