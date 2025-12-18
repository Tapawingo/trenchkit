#ifndef MODINFO_H
#define MODINFO_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QUuid>

struct ModInfo {
    QString id;                    // Unique identifier (UUID)
    QString name;                  // Mod display name
    QString fileName;              // Original .pak file name
    QString numberedFileName;      // Numbered name in paks folder (e.g., "001_modname.pak")
    QDateTime installDate;         // When the mod was added
    bool enabled;                  // Whether mod is currently active
    int priority;                  // Load order (lower = loads first)
    QString nexusModId;           // Optional NexusMods ID
    QString version;              // Optional mod version
    QString author;               // Optional mod author
    QString description;          // Optional mod description

    // Constructor
    ModInfo()
        : enabled(false)
        , priority(0)
    {}

    // Serialization
    QJsonObject toJson() const;
    static ModInfo fromJson(const QJsonObject &json);

    // Generate unique ID
    static QString generateId() {
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
};

#endif // MODINFO_H
