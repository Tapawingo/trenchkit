#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QTranslator>

class TranslationManager : public QObject {
    Q_OBJECT

public:
    static TranslationManager &instance();

    void initialize();
    void setLanguage(const QString &language);
    QString currentLanguage() const { return m_currentLanguage; }

private:
    explicit TranslationManager(QObject *parent = nullptr);

    void removeTranslators();
    bool installTranslators(const QString &locale);

    QTranslator *m_appTranslator = nullptr;
    QTranslator *m_qtTranslator = nullptr;
    QString m_currentLanguage;
};

#endif // TRANSLATIONMANAGER_H
