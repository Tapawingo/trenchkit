#include "TsTranslator.h"

#include <QFile>
#include <QXmlStreamReader>

TsTranslator::TsTranslator(QObject *parent)
    : QTranslator(parent) {
}

bool TsTranslator::loadTsFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    m_messages.clear();
    m_language.clear();

    QXmlStreamReader xml(&file);
    QString currentContext;

    while (!xml.atEnd() && !xml.hasError()) {
        auto token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == QStringLiteral("TS")) {
                m_language = xml.attributes().value(QStringLiteral("language")).toString();

            } else if (xml.name() == QStringLiteral("context")) {
                currentContext.clear();

            } else if (xml.name() == QStringLiteral("name") && !currentContext.isEmpty()) {
                // Already parsed context name
            } else if (xml.name() == QStringLiteral("name")) {
                currentContext = xml.readElementText();

            } else if (xml.name() == QStringLiteral("message")) {
                bool isPlural = xml.attributes().value(QStringLiteral("numerus")) == QStringLiteral("yes");
                QString source;
                QString translationText;
                QString disambiguation;
                QStringList pluralForms;
                bool isUnfinished = false;

                while (!(xml.readNext() == QXmlStreamReader::EndElement
                         && xml.name() == QStringLiteral("message"))) {
                    if (xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == QStringLiteral("source")) {
                            source = xml.readElementText();
                        } else if (xml.name() == QStringLiteral("comment")) {
                            disambiguation = xml.readElementText();
                        } else if (xml.name() == QStringLiteral("translation")) {
                            isUnfinished = xml.attributes().value(QStringLiteral("type"))
                                           == QStringLiteral("unfinished");
                            if (isPlural) {
                                // Read numerusform children
                                while (!(xml.readNext() == QXmlStreamReader::EndElement
                                         && xml.name() == QStringLiteral("translation"))) {
                                    if (xml.tokenType() == QXmlStreamReader::StartElement
                                        && xml.name() == QStringLiteral("numerusform")) {
                                        pluralForms.append(xml.readElementText());
                                    }
                                }
                            } else {
                                translationText = xml.readElementText();
                            }
                        }
                    }
                    if (xml.hasError()) break;
                }

                if (isUnfinished && translationText.isEmpty() && pluralForms.isEmpty()) {
                    continue;
                }

                MessageKey key{currentContext, source, disambiguation};
                MessageValue value;
                if (isPlural) {
                    value.isPlural = true;
                    value.pluralForms = pluralForms;
                } else {
                    value.translation = translationText;
                }
                m_messages.insert(key, value);
            }
        }
    }

    if (xml.hasError()) {
        m_messages.clear();
        m_language.clear();
        return false;
    }

    return !m_messages.isEmpty();
}

QString TsTranslator::translate(const char *context, const char *sourceText,
                                const char *disambiguation, int n) const {
    MessageKey key{
        QString::fromUtf8(context),
        QString::fromUtf8(sourceText),
        disambiguation ? QString::fromUtf8(disambiguation) : QString()
    };

    auto it = m_messages.constFind(key);
    if (it == m_messages.constEnd()) {
        return {};
    }

    const auto &value = it.value();
    if (value.isPlural && n >= 0) {
        if (value.pluralForms.isEmpty()) {
            return {};
        }
        int form = (n == 1) ? 0 : 1;
        if (form >= value.pluralForms.size()) {
            form = value.pluralForms.size() - 1;
        }
        QString result = value.pluralForms.at(form);
        result.replace(QStringLiteral("%n"), QString::number(n));
        return result;
    }

    return value.translation;
}

bool TsTranslator::isEmpty() const {
    return m_messages.isEmpty();
}

QString TsTranslator::language() const {
    return m_language;
}
