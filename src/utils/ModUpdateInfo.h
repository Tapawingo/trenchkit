#ifndef MODUPDATEINFO_H
#define MODUPDATEINFO_H

#include <QString>
#include <QDateTime>

struct ModUpdateInfo {
    QString modId;
    QString currentVersion;
    QString availableVersion;
    QString availableFileId;
    QDateTime lastChecked;
    bool updateAvailable = false;

    ModUpdateInfo() = default;

    ModUpdateInfo(const QString &id, const QString &current, const QString &available,
                  const QString &fileId)
        : modId(id)
        , currentVersion(current)
        , availableVersion(available)
        , availableFileId(fileId)
        , lastChecked(QDateTime::currentDateTime())
        , updateAvailable(true)
    {}
};

#endif // MODUPDATEINFO_H
