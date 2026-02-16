#include "ModManifestReader.h"

#include "PakFileReader.h"

#include <QXmlStreamReader>

namespace {
QString normalizeNoticeIcon(const QString &icon) {
    QString lower = icon.trimmed().toLower();
    if (lower == QStringLiteral("warning") ||
        lower == QStringLiteral("error") ||
        lower == QStringLiteral("question") ||
        lower == QStringLiteral("lightbulb") ||
        lower == QStringLiteral("info")) {
        return lower;
    }
    return QStringLiteral("info");
}
}

bool ModManifestReader::readFromPak(const QString &pakPath, ModManifest *manifest, QString *error) {
    if (!manifest) {
        if (error) {
            *error = QStringLiteral("No manifest output provided");
        }
        return false;
    }

    QByteArray data;
    QStringList candidates;
    candidates << QStringLiteral("Mod/Manifest.xml");

    QString pakError;
    if (!PakFileReader::extractFile(pakPath, candidates, &data, &pakError)) {
        if (error) {
            *error = pakError;
        }
        return false;
    }

    return parseXml(data, manifest, error);
}

bool ModManifestReader::parseXml(const QByteArray &data, ModManifest *manifest, QString *error) {
    if (!manifest) {
        if (error) {
            *error = QStringLiteral("No manifest output provided");
        }
        return false;
    }

    QXmlStreamReader xml(data);
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement()) {
            continue;
        }

        const QStringView elementName = xml.name();
        if (elementName == QStringLiteral("ModManifest")) {
            continue;
        }

        if (elementName == QStringLiteral("Id")) {
            manifest->id = xml.readElementText().trimmed();
            continue;
        }

        if (elementName == QStringLiteral("Name")) {
            manifest->name = xml.readElementText().trimmed();
            continue;
        }

        if (elementName == QStringLiteral("Version")) {
            manifest->version = xml.readElementText().trimmed();
            continue;
        }

        if (elementName == QStringLiteral("Author")) {
            const QString author = xml.readElementText().trimmed();
            if (!author.isEmpty()) {
                manifest->authors.append(author);
            }
            continue;
        }

        if (elementName == QStringLiteral("Description")) {
            manifest->description = xml.readElementText().trimmed();
            continue;
        }

        if (elementName == QStringLiteral("Homepage")) {
            manifest->homepageUrl = xml.readElementText().trimmed();
            continue;
        }

        if (elementName == QStringLiteral("Nexus")) {
            manifest->nexusUrl = xml.readElementText().trimmed();
            continue;
        }

        if (elementName == QStringLiteral("Itch")) {
            manifest->itchUrl = xml.readElementText().trimmed();
            continue;
        }

        if (elementName == QStringLiteral("Notice")) {
            manifest->noticeIcon = normalizeNoticeIcon(xml.attributes().value(QStringLiteral("icon")).toString());
            manifest->noticeText = xml.readElementText().trimmed();
            continue;
        }

        if (elementName == QStringLiteral("Dependency")) {
            ModManifest::Dependency dep;
            dep.id = xml.attributes().value(QStringLiteral("id")).toString().trimmed();
            dep.minVersion = xml.attributes().value(QStringLiteral("minVersion")).toString().trimmed();
            dep.maxVersion = xml.attributes().value(QStringLiteral("maxVersion")).toString().trimmed();
            const QString requiredAttr = xml.attributes().value(QStringLiteral("required")).toString().trimmed().toLower();
            if (!requiredAttr.isEmpty()) {
                dep.required = (requiredAttr != QStringLiteral("false") && requiredAttr != QStringLiteral("0"));
            }
            if (!dep.id.isEmpty()) {
                manifest->dependencies.append(dep);
            }
            xml.skipCurrentElement();
            continue;
        }

        if (elementName == QStringLiteral("Tag")) {
            const QString tag = xml.readElementText().trimmed();
            if (!tag.isEmpty()) {
                manifest->tags.append(tag);
            }
            continue;
        }
    }

    if (xml.hasError()) {
        if (error) {
            *error = xml.errorString();
        }
        return false;
    }

    return !manifest->id.isEmpty() || !manifest->name.isEmpty();
}
