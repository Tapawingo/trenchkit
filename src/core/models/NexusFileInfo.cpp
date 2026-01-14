#include "NexusFileInfo.h"
#include <QJsonArray>

NexusFileInfo NexusFileInfo::fromJson(const QJsonObject &json) {
    NexusFileInfo info;

    QJsonValue idValue = json.value("id");
    if (idValue.isArray()) {
        QJsonArray idArray = idValue.toArray();
        if (!idArray.isEmpty()) {
            info.id = QString::number(idArray.first().toInt());
        }
    } else {
        info.id = QString::number(idValue.toInt());
    }

    info.name = json.value("name").toString();
    info.version = json.value("version").toString();
    info.description = json.value("description").toString();
    info.sizeBytes = json.value("size").toInteger();
    info.categoryName = json.value("category_name").toString();

    qint64 timestamp = json.value("uploaded_timestamp").toInteger();
    if (timestamp > 0) {
        info.uploadedTime = QDateTime::fromSecsSinceEpoch(timestamp);
    }

    return info;
}
