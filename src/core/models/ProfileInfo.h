#ifndef PROFILEINFO_H
#define PROFILEINFO_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QList>

struct ModConfig {
    QString modId;
    bool enabled;
    int priority;

    QJsonObject toJson() const;
    static ModConfig fromJson(const QJsonObject &json);
};

struct ProfileInfo {
    QString id;
    QString name;
    QDateTime createdDate;
    QDateTime modifiedDate;
    QList<ModConfig> modConfigs;

    ProfileInfo()
        : createdDate(QDateTime::currentDateTime())
        , modifiedDate(QDateTime::currentDateTime())
    {}

    QJsonObject toJson() const;
    static ProfileInfo fromJson(const QJsonObject &json);

    static QString generateId() {
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
};

#endif // PROFILEINFO_H
