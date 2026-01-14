#pragma once

#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <QList>
#include <QPair>

struct NexusFileInfo {
    QString id;
    QString name;
    QString version;
    QString description;
    qint64 sizeBytes = -1;
    QString categoryName;
    QDateTime uploadedTime;
    QList<QPair<QString, QString>> fileUpdates;

    static NexusFileInfo fromJson(const QJsonObject &json);
};
