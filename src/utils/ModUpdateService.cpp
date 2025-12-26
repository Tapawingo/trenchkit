#include "ModUpdateService.h"
#include "ModManager.h"
#include "NexusModsClient.h"
#include "NexusFileInfo.h"
#include <QVersionNumber>
#include <QDebug>

ModUpdateService::ModUpdateService(ModManager *modManager,
                                   NexusModsClient *nexusClient,
                                   QObject *parent)
    : QObject(parent)
    , m_modManager(modManager)
    , m_nexusClient(nexusClient)
{
    connect(m_nexusClient, &NexusModsClient::modFilesReceived,
            this, &ModUpdateService::onModFilesReceived);
    connect(m_nexusClient, &NexusModsClient::errorOccurred,
            this, &ModUpdateService::onError);

    m_rateLimitTimer.setSingleShot(true);
    connect(&m_rateLimitTimer, &QTimer::timeout, this, &ModUpdateService::processNextMod);
}

bool ModUpdateService::hasUpdate(const QString &modId) const {
    return m_updateCache.contains(modId) && m_updateCache[modId].updateAvailable;
}

ModUpdateInfo ModUpdateService::getUpdateInfo(const QString &modId) const {
    return m_updateCache.value(modId);
}

void ModUpdateService::checkAllModsForUpdates() {
    if (m_isChecking) {
        qDebug() << "Update check already in progress";
        return;
    }

    if (!m_modManager || !m_nexusClient) {
        qDebug() << "ModUpdateService: Missing dependencies";
        return;
    }

    m_updateCache.clear();
    m_modsToCheck.clear();
    m_currentModIndex = 0;
    m_updatesFound = 0;

    QList<ModInfo> allMods = m_modManager->getMods();
    for (const ModInfo &mod : allMods) {
        if (!mod.nexusModId.isEmpty() && !mod.nexusFileId.isEmpty() && !mod.version.isEmpty()) {
            m_modsToCheck.append(mod);
        }
    }

    m_totalMods = m_modsToCheck.size();

    if (m_totalMods == 0) {
        emit checkComplete(0);
        return;
    }

    m_isChecking = true;
    emit checkStarted();
    emit checkProgress(0, m_totalMods);

    processNextMod();
}

void ModUpdateService::checkModForUpdate(const QString &modId) {
    if (!m_modManager || !m_nexusClient) {
        return;
    }

    ModInfo mod = m_modManager->getMod(modId);
    if (mod.id.isEmpty() || mod.nexusModId.isEmpty() || mod.version.isEmpty()) {
        return;
    }

    m_nexusClient->getModFiles(mod.nexusModId);
}

void ModUpdateService::cancelCheck() {
    m_isChecking = false;
    m_rateLimitTimer.stop();
    m_modsToCheck.clear();
    m_currentModIndex = 0;
    qDebug() << "Update check cancelled";
}

void ModUpdateService::processNextMod() {
    if (!m_isChecking || m_currentModIndex >= m_modsToCheck.size()) {
        if (m_isChecking) {
            m_isChecking = false;
            emit checkComplete(m_updatesFound);
            qDebug() << "Update check complete. Found" << m_updatesFound << "updates";
        }
        return;
    }

    const ModInfo &mod = m_modsToCheck[m_currentModIndex];
    emit checkProgress(m_currentModIndex + 1, m_totalMods);

    qDebug() << "Checking for updates:" << mod.name << "version" << mod.version;
    m_nexusClient->getModFiles(mod.nexusModId);
}

void ModUpdateService::onModFilesReceived(const QList<NexusFileInfo> &files) {
    if (!m_isChecking || m_currentModIndex >= m_modsToCheck.size()) {
        return;
    }

    const ModInfo &mod = m_modsToCheck[m_currentModIndex];

    qDebug() << "Mod:" << mod.name << "| Current file ID:" << mod.nexusFileId << "| Version:" << mod.version;

    QString latestVersion;
    QString latestFileId;
    QDateTime latestDate;

    bool isExplicitUpdate = false;
    findLatestVersion(files, mod.nexusFileId, latestVersion, latestFileId, latestDate, isExplicitUpdate);

    if (!latestVersion.isEmpty() && !latestFileId.isEmpty()) {
        bool shouldUpdate = isExplicitUpdate || isUpdateAvailable(mod.version, latestVersion);

        if (shouldUpdate) {
            ModUpdateInfo updateInfo(mod.id, mod.version, latestVersion, latestFileId);
            m_updateCache[mod.id] = updateInfo;
            m_updatesFound++;

            qDebug() << "Update available for" << mod.name << ":"
                     << mod.version << "->" << latestVersion
                     << (isExplicitUpdate ? "(explicit)" : "(version)");
            emit updateFound(mod.id, updateInfo);
        } else {
            qDebug() << "No update for" << mod.name << "(current:" << mod.version
                     << "latest:" << latestVersion << ")";
        }
    }

    m_currentModIndex++;
    m_rateLimitTimer.start(RATE_LIMIT_DELAY_MS);
}

void ModUpdateService::onError(const QString &error) {
    if (!m_isChecking) {
        return;
    }

    qDebug() << "Update check error:" << error;

    if (error == QStringLiteral("PREMIUM_REQUIRED") || error.contains(QStringLiteral("429"))) {
        qDebug() << "Stopping update check due to rate limiting or premium requirement";
        m_isChecking = false;
        emit errorOccurred(error);
        emit checkComplete(m_updatesFound);
        return;
    }

    m_currentModIndex++;
    m_rateLimitTimer.start(RATE_LIMIT_DELAY_MS);
}

bool ModUpdateService::isUpdateAvailable(const QString &currentVersion,
                                         const QString &availableVersion) const {
    QVersionNumber current = QVersionNumber::fromString(currentVersion);
    QVersionNumber available = QVersionNumber::fromString(availableVersion);

    if (current.isNull() || available.isNull()) {
        return false;
    }

    return QVersionNumber::compare(available, current) > 0;
}

void ModUpdateService::findLatestVersion(const QList<NexusFileInfo> &files,
                                         const QString &currentFileId,
                                         QString &latestVersion,
                                         QString &latestFileId,
                                         QDateTime &latestDate,
                                         bool &isExplicit) const {
    latestVersion.clear();
    latestFileId.clear();
    latestDate = QDateTime();
    isExplicit = false;

    qDebug() << "Looking for updates for file ID:" << currentFileId;

    QList<NexusFileInfo> explicitUpdates;
    for (const NexusFileInfo &file : files) {
        if (file.categoryName == QStringLiteral("ARCHIVED")) {
            continue;
        }

        for (const auto &update : file.fileUpdates) {
            if (update.first == currentFileId) {
                qDebug() << "  Found explicit update: file" << file.id << "replaces" << currentFileId;
                explicitUpdates.append(file);
                break;
            }
        }
    }

    if (!explicitUpdates.isEmpty()) {
        qDebug() << "  Using" << explicitUpdates.size() << "explicit update(s)";
        for (const NexusFileInfo &file : explicitUpdates) {
            if (file.uploadedTime > latestDate) {
                latestVersion = file.version;
                latestFileId = file.id;
                latestDate = file.uploadedTime;
            }
        }
        qDebug() << "  Selected file" << latestFileId << "version" << latestVersion;
        isExplicit = true;
        return;
    }

    qDebug() << "  No explicit updates found, using fallback logic";

    for (const NexusFileInfo &file : files) {
        if (file.categoryName == QStringLiteral("ARCHIVED")) {
            continue;
        }

        bool isPreferred = (file.categoryName == QStringLiteral("MAIN") ||
                           file.categoryName == QStringLiteral("UPDATE"));

        if (isPreferred && file.uploadedTime > latestDate) {
            latestVersion = file.version;
            latestFileId = file.id;
            latestDate = file.uploadedTime;
        }
    }

    if (latestVersion.isEmpty() && !files.isEmpty()) {
        for (const NexusFileInfo &file : files) {
            if (file.categoryName != QStringLiteral("ARCHIVED") &&
                file.uploadedTime > latestDate) {
                latestVersion = file.version;
                latestFileId = file.id;
                latestDate = file.uploadedTime;
            }
        }
    }
}
