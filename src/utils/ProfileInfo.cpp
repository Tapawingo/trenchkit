#include "ProfileInfo.h"

QJsonObject ModConfig::toJson() const {
    QJsonObject json;
    json["modId"] = modId;
    json["enabled"] = enabled;
    json["priority"] = priority;
    return json;
}

ModConfig ModConfig::fromJson(const QJsonObject &json) {
    ModConfig config;
    config.modId = json["modId"].toString();
    config.enabled = json["enabled"].toBool();
    config.priority = json["priority"].toInt();
    return config;
}

QJsonObject ProfileInfo::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["name"] = name;
    json["createdDate"] = createdDate.toString(Qt::ISODate);
    json["modifiedDate"] = modifiedDate.toString(Qt::ISODate);

    QJsonArray modsArray;
    for (const ModConfig &config : modConfigs) {
        modsArray.append(config.toJson());
    }
    json["mods"] = modsArray;

    return json;
}

ProfileInfo ProfileInfo::fromJson(const QJsonObject &json) {
    ProfileInfo profile;
    profile.id = json["id"].toString();
    profile.name = json["name"].toString();
    profile.createdDate = QDateTime::fromString(json["createdDate"].toString(), Qt::ISODate);
    profile.modifiedDate = QDateTime::fromString(json["modifiedDate"].toString(), Qt::ISODate);

    QJsonArray modsArray = json["mods"].toArray();
    for (const QJsonValue &value : modsArray) {
        ModConfig config = ModConfig::fromJson(value.toObject());
        profile.modConfigs.append(config);
    }

    return profile;
}
