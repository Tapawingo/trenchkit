#include "ModInfo.h"

QJsonObject ModInfo::toJson() const {
    QJsonObject json;
    json["id"] = id;
    json["name"] = name;
    json["fileName"] = fileName;
    json["numberedFileName"] = numberedFileName;
    json["installDate"] = installDate.toString(Qt::ISODate);
    json["enabled"] = enabled;
    json["priority"] = priority;

    // Optional fields
    if (!nexusModId.isEmpty()) {
        json["nexusModId"] = nexusModId;
    }
    if (!nexusFileId.isEmpty()) {
        json["nexusFileId"] = nexusFileId;
    }
    if (!itchGameId.isEmpty()) {
        json["itchGameId"] = itchGameId;
    }
    if (!version.isEmpty()) {
        json["version"] = version;
    }
    if (!author.isEmpty()) {
        json["author"] = author;
    }
    if (!description.isEmpty()) {
        json["description"] = description;
    }

    return json;
}

ModInfo ModInfo::fromJson(const QJsonObject &json) {
    ModInfo mod;
    mod.id = json["id"].toString();
    mod.name = json["name"].toString();
    mod.fileName = json["fileName"].toString();
    mod.numberedFileName = json["numberedFileName"].toString();
    mod.installDate = QDateTime::fromString(json["installDate"].toString(), Qt::ISODate);
    mod.enabled = json["enabled"].toBool();
    mod.priority = json["priority"].toInt();

    // Optional fields
    if (json.contains("nexusModId")) {
        mod.nexusModId = json["nexusModId"].toString();
    }
    if (json.contains("nexusFileId")) {
        mod.nexusFileId = json["nexusFileId"].toString();
    }
    if (json.contains("itchGameId")) {
        mod.itchGameId = json["itchGameId"].toString();
    }
    if (json.contains("version")) {
        mod.version = json["version"].toString();
    }
    if (json.contains("author")) {
        mod.author = json["author"].toString();
    }
    if (json.contains("description")) {
        mod.description = json["description"].toString();
    }

    return mod;
}
