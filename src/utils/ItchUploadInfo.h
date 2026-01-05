#ifndef ITCHUPLOADINFO_H
#define ITCHUPLOADINFO_H

#include <QString>
#include <QJsonObject>
#include <QDateTime>

struct ItchUploadInfo {
    QString id;              // Upload ID (numeric)
    QString filename;        // Original filename
    QString displayName;     // Display name (may be empty)
    qint64 sizeBytes = -1;   // File size in bytes
    QDateTime createdAt;     // Upload timestamp
    QDateTime updatedAt;     // Last update timestamp

    static ItchUploadInfo fromJson(const QJsonObject &json);
};

#endif // ITCHUPLOADINFO_H
