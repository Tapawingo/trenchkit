#ifndef MODMANIFESTREADER_H
#define MODMANIFESTREADER_H

#include <QString>
#include <QStringList>
#include <QList>

struct ModManifest {
    struct Dependency {
        QString id;
        QString minVersion;
        QString maxVersion;
        bool required = true;
    };

    QString id;
    QString name;
    QString version;
    QString description;
    QString homepageUrl;
    QString nexusUrl;
    QString itchUrl;
    QString noticeText;
    QString noticeIcon;
    QStringList authors;
    QList<Dependency> dependencies;
    QStringList tags;
};

class ModManifestReader {
public:
    static bool readFromPak(const QString &pakPath, ModManifest *manifest, QString *error = nullptr);
    static bool parseXml(const QByteArray &data, ModManifest *manifest, QString *error = nullptr);
};

#endif // MODMANIFESTREADER_H
