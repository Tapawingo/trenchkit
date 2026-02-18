#include "TranslationManager.h"
#include "TsTranslator.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QSet>
#include <QSettings>

TranslationManager &TranslationManager::instance() {
    static TranslationManager s_instance;
    return s_instance;
}

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent) {
}

void TranslationManager::initialize() {
    discoverLanguages();

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

QString TranslationManager::localesPath() const {
    return QCoreApplication::applicationDirPath() + QStringLiteral("/locales");
}

void TranslationManager::discoverLanguages() {
    m_availableLanguages.clear();
    QSet<QString> seen;

    // Scan embedded .qm resources
    QDir resourceDir(QStringLiteral(":/i18n"));
    const auto entries = resourceDir.entryList({QStringLiteral("TrenchKit_*.qm")}, QDir::Files);
    for (const auto &entry : entries) {
        // Extract code from "TrenchKit_<code>.qm"
        QString code = entry.mid(10); // skip "TrenchKit_"
        code.chop(3); // remove ".qm"
        if (!code.isEmpty() && code != QStringLiteral("en") && !seen.contains(code)) {
            seen.insert(code);
        }
    }

    // Scan external .ts files
    QDir localesDir(localesPath());
    if (localesDir.exists()) {
        const auto tsEntries = localesDir.entryList({QStringLiteral("TrenchKit_*.ts")}, QDir::Files);
        for (const auto &entry : tsEntries) {
            QString code = entry.mid(10);
            code.chop(3); // remove ".ts"
            if (!code.isEmpty() && code != QStringLiteral("en") && !seen.contains(code)) {
                seen.insert(code);
            }
        }
    }

    // Build sorted list with display names
    QStringList codes(seen.begin(), seen.end());
    codes.sort();

    for (const auto &code : codes) {
        QLocale locale(code);
        QString displayName = locale.nativeLanguageName();
        if (!displayName.isEmpty()) {
            displayName[0] = displayName[0].toUpper();
        }
        m_availableLanguages.append({code, displayName});
    }
}

void TranslationManager::removeTranslators() {
    auto *app = qApp;
    if (!app) return;

    if (m_tsTranslator) {
        app->removeTranslator(m_tsTranslator);
        delete m_tsTranslator;
        m_tsTranslator = nullptr;
    }
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

    // Qt base translations
    m_qtTranslator = new QTranslator(this);
    if (m_qtTranslator->load(QLocale(locale), QStringLiteral("qtbase"), QStringLiteral("_"),
                              QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app->installTranslator(m_qtTranslator);
    } else {
        delete m_qtTranslator;
        m_qtTranslator = nullptr;
    }

    // Try external .ts file first
    QString tsPath = localesPath() + QStringLiteral("/TrenchKit_") + locale + QStringLiteral(".ts");
    if (QFile::exists(tsPath)) {
        m_tsTranslator = new TsTranslator(this);
        if (m_tsTranslator->loadTsFile(tsPath)) {
            app->installTranslator(m_tsTranslator);
            return true;
        }
        delete m_tsTranslator;
        m_tsTranslator = nullptr;
    }

    // Fall back to embedded .qm
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
