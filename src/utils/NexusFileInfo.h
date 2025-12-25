#pragma once

#include <QString>
#include <QJsonObject>
#include <QDateTime>

struct NexusFileInfo {
    QString id;
    QString name;
    QString version;
    QString description;
    qint64 sizeBytes = -1;
    QString categoryName;
    QDateTime uploadedTime;

    static NexusFileInfo fromJson(const QJsonObject &json);
};
