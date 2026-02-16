#ifndef MODINFO_H
#define MODINFO_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QUuid>

struct ModInfo {
    struct Dependency {
        QString id;
        QString minVersion;
        QString maxVersion;
        bool required = true;
    };

    QString id;                    // Unique identifier (UUID)
    QString name;                  // Mod display name
    QString fileName;              // Original .pak file name
    QString numberedFileName;      // Numbered name in paks folder (e.g., "001_modname.pak")
    QDateTime installDate;         // When the mod was added
    QDateTime uploadDate;          // When the mod was uploaded (Nexus/itch.io)
    bool enabled;                  // Whether mod is currently active
    int priority;                  // Load order (lower = loads first)
    QString nexusModId;            // Optional Nexus Mod ID
    QString nexusFileId;           // Optional Nexus File ID
    QString nexusUrl;              // Optional Nexus Mods URL
    QString itchGameId;            // Optional itch.io Game ID
    QString itchUrl;               // Optional itch.io URL
    QString version;               // Optional mod version
    QString author;                // Optional mod author
    QString description;           // Optional mod description
    QString homepageUrl;           // Optional mod homepage URL
    QString manifestId;            // Optional manifest ID
    QStringList manifestAuthors;   // Optional manifest authors
    QList<Dependency> manifestDependencies; // Optional manifest dependencies
    QStringList manifestTags;      // Optional manifest tags
    QString noticeText;            // Optional notice text
    QString noticeIcon;            // Optional notice icon type
    QStringList ignoredItchUploadIds; // List of itch.io upload IDs that should be ignored for updates

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
