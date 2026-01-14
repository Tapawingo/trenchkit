#include "ItchUploadInfo.h"

ItchUploadInfo ItchUploadInfo::fromJson(const QJsonObject &json) {
    ItchUploadInfo info;

    // Upload ID (convert to string)
    if (json.contains("id")) {
        info.id = QString::number(json["id"].toInt());
    }

    // Filename
    if (json.contains("filename")) {
        info.filename = json["filename"].toString();
    }

    // Display name (optional)
    if (json.contains("display_name")) {
        info.displayName = json["display_name"].toString();
    }

    // Size in bytes
    if (json.contains("size")) {
        info.sizeBytes = json["size"].toVariant().toLongLong();
    }

    // Created timestamp - itch.io uses format: "YYYY-MM-DD HH:MM:SS"
    if (json.contains("created_at")) {
        QString createdStr = json["created_at"].toString();
        info.createdAt = QDateTime::fromString(createdStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    }

    // Updated timestamp
    if (json.contains("updated_at")) {
        QString updatedStr = json["updated_at"].toString();
        info.updatedAt = QDateTime::fromString(updatedStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    }

    return info;
}
