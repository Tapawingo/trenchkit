#ifndef ITCHUPDATEINFO_H
#define ITCHUPDATEINFO_H

#include <QString>
#include <QDateTime>

struct ItchUpdateInfo {
    QString modId;
    QString currentVersion;
    QString availableVersion;
    QString availableUploadId;
    QDateTime currentUploadDate;
    QDateTime availableUploadDate;
    QDateTime lastChecked;
    bool updateAvailable = false;

    ItchUpdateInfo() = default;

    ItchUpdateInfo(const QString &id, const QString &current, const QString &available,
                   const QString &uploadId, const QDateTime &currentDate, const QDateTime &availableDate)
        : modId(id)
        , currentVersion(current)
        , availableVersion(available)
        , availableUploadId(uploadId)
        , currentUploadDate(currentDate)
        , availableUploadDate(availableDate)
        , lastChecked(QDateTime::currentDateTime())
        , updateAvailable(true)
    {}
};

#endif
