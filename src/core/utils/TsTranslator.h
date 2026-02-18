#ifndef TSTRANSLATOR_H
#define TSTRANSLATOR_H

#include <QHash>
#include <QString>
#include <QTranslator>

class TsTranslator : public QTranslator {
    Q_OBJECT

public:
    explicit TsTranslator(QObject *parent = nullptr);

    bool loadTsFile(const QString &filePath);
    QString translate(const char *context, const char *sourceText,
                      const char *disambiguation = nullptr, int n = -1) const override;
    bool isEmpty() const override;
    QString language() const;

private:
    struct MessageKey {
        QString context;
        QString source;
        QString disambiguation;

        bool operator==(const MessageKey &other) const = default;
    };

    struct MessageValue {
        QString translation;
        QStringList pluralForms;
        bool isPlural = false;
    };

    friend size_t qHash(const TsTranslator::MessageKey &key, size_t seed);

    QHash<MessageKey, MessageValue> m_messages;
    QString m_language;
};

inline size_t qHash(const TsTranslator::MessageKey &key, size_t seed = 0) {
    return qHash(key.context, seed) ^ qHash(key.source, seed) ^ qHash(key.disambiguation, seed);
}

#endif // TSTRANSLATOR_H
