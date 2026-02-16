#include "TranslationManager.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>

TranslationManager &TranslationManager::instance() {
    static TranslationManager s_instance;
    return s_instance;
}

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent) {
}

void TranslationManager::initialize() {
    QSettings settings(QStringLiteral("TrenchKit"), QStringLiteral("FoxholeModManager"));
    QString language = settings.value(QStringLiteral("app/language"), QStringLiteral("system")).toString();
    setLanguage(language);
}

void TranslationManager::setLanguage(const QString &language) {
    removeTranslators();
    m_currentLanguage = language;

    QString locale;
    if (language == QStringLiteral("system")) {
        locale = QLocale::system().name();
    } else if (language == QStringLiteral("en")) {
        return;
    } else {
        locale = language;
    }

    installTranslators(locale);
}

void TranslationManager::removeTranslators() {
    auto *app = qApp;
    if (!app) return;

    if (m_appTranslator) {
        app->removeTranslator(m_appTranslator);
        delete m_appTranslator;
        m_appTranslator = nullptr;
    }
    if (m_qtTranslator) {
        app->removeTranslator(m_qtTranslator);
        delete m_qtTranslator;
        m_qtTranslator = nullptr;
    }
}

bool TranslationManager::installTranslators(const QString &locale) {
    auto *app = qApp;
    if (!app) return false;

    m_qtTranslator = new QTranslator(this);
    if (m_qtTranslator->load(QLocale(locale), QStringLiteral("qtbase"), QStringLiteral("_"),
                              QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app->installTranslator(m_qtTranslator);
    } else {
        delete m_qtTranslator;
        m_qtTranslator = nullptr;
    }

    m_appTranslator = new QTranslator(this);
    if (m_appTranslator->load(QLocale(locale), QStringLiteral("TrenchKit"), QStringLiteral("_"),
                               QStringLiteral(":/i18n"))) {
        app->installTranslator(m_appTranslator);
        return true;
    }

    delete m_appTranslator;
    m_appTranslator = nullptr;
    return false;
}
