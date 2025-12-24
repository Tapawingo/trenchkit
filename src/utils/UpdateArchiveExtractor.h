#pragma once

#include <QString>

class UpdateArchiveExtractor {
public:
    static bool extractZip(const QString &zipPath, const QString &destDir, QString *error);
};
