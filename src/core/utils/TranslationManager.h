#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QList>
#include <QObject>
#include <QTranslator>

class TsTranslator;

class TranslationManager : public QObject {
    Q_OBJECT

public:
    struct LanguageInfo {
        QString code;
        QString displayName;
    };

    static TranslationManager &instance();

    void initialize();
    void setLanguage(const QString &language);
    QString currentLanguage() const { return m_currentLanguage; }
    QList<LanguageInfo> availableLanguages() const { return m_availableLanguages; }
    QString localesPath() const;

private:
    explicit TranslationManager(QObject *parent = nullptr);

    void discoverLanguages();
    void removeTranslators();
    bool installTranslators(const QString &locale);

    QTranslator *m_appTranslator = nullptr;
    QTranslator *m_qtTranslator = nullptr;
    TsTranslator *m_tsTranslator = nullptr;
    QString m_currentLanguage;
    QList<LanguageInfo> m_availableLanguages;
};

#endif // TRANSLATIONMANAGER_H
