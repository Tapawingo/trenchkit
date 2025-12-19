#include "ProfileManager.h"
#include "ModManager.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDebug>
#include <QFileInfo>

QString ProfileValidationResult::getMessage() const {
    if (isValid && !hasMissingMods()) {
        return "Profile is valid and all mods are available.";
    }

    QString msg;
    if (hasMissingMods()) {
        msg += QString("%1 of %2 mods are missing:\n")
            .arg(missingMods.size())
            .arg(totalModsCount);

        int autoInstallCount = 0;
        for (const MissingModInfo &info : missingMods) {
            if (info.hasNexusId()) {
                autoInstallCount++;
            }
        }

        if (autoInstallCount > 0) {
            msg += QString("\n%1 mod(s) have NexusMods IDs (available for auto-install).")
                .arg(autoInstallCount);
        }
    }

    return msg;
}

ProfileManager::ProfileManager(QObject *parent)
    : QObject(parent)
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_storagePath = appDataPath + "/mods";
    QDir().mkpath(m_storagePath);
}

void ProfileManager::setModManager(ModManager *modManager) {
    m_modManager = modManager;
}

void ProfileManager::setStoragePath(const QString &storagePath) {
    m_storagePath = storagePath;
    QDir().mkpath(m_storagePath);
}

bool ProfileManager::createProfile(const QString &name) {
    if (!m_modManager) {
        emit errorOccurred("ModManager not set");
        return false;
    }

    if (name.trimmed().isEmpty()) {
        emit errorOccurred("Profile name cannot be empty");
        return false;
    }

    for (const ProfileInfo &profile : m_profiles) {
        if (profile.name == name) {
            emit errorOccurred("Profile with this name already exists");
            return false;
        }
    }

    ProfileInfo profile = captureCurrentState();
    profile.id = ProfileInfo::generateId();
    profile.name = name;

    m_profiles.append(profile);
    saveProfiles();
    setActiveProfile(profile.id);

    emit profileCreated(profile.id);
    emit profilesChanged();

    qDebug() << "Created profile:" << name << "with" << profile.modConfigs.size() << "mods";
    return true;
}

bool ProfileManager::updateProfile(const QString &profileId) {
    if (!m_modManager) {
        emit errorOccurred("ModManager not set");
        return false;
    }

    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&profileId](const ProfileInfo &p) { return p.id == profileId; });

    if (it == m_profiles.end()) {
        emit errorOccurred("Profile not found: " + profileId);
        return false;
    }

    ProfileInfo currentState = captureCurrentState();
    it->modConfigs = currentState.modConfigs;
    it->modifiedDate = QDateTime::currentDateTime();

    saveProfiles();
    emit profilesChanged();

    qDebug() << "Updated profile:" << it->name;
    return true;
}

bool ProfileManager::deleteProfile(const QString &profileId) {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&profileId](const ProfileInfo &p) { return p.id == profileId; });

    if (it == m_profiles.end()) {
        emit errorOccurred("Profile not found: " + profileId);
        return false;
    }

    QString name = it->name;
    m_profiles.erase(it);

    if (m_activeProfileId == profileId) {
        clearActiveProfile();
    }

    saveProfiles();
    emit profileDeleted(profileId);
    emit profilesChanged();

    qDebug() << "Deleted profile:" << name;
    return true;
}

ProfileInfo ProfileManager::getProfile(const QString &profileId) const {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&profileId](const ProfileInfo &p) { return p.id == profileId; });
    return it != m_profiles.end() ? *it : ProfileInfo();
}

QList<ProfileInfo> ProfileManager::getProfiles() const {
    return m_profiles;
}

ProfileValidationResult ProfileManager::validateProfile(const QString &profileId) const {
    ProfileValidationResult result;
    result.isValid = false;

    if (!m_modManager) {
        return result;
    }

    ProfileInfo profile = getProfile(profileId);
    if (profile.id.isEmpty()) {
        return result;
    }

    QList<ModInfo> availableMods = m_modManager->getMods();
    result.totalModsCount = profile.modConfigs.size();
    result.availableModsCount = 0;

    for (const ModConfig &config : profile.modConfigs) {
        bool found = false;
        QString nexusId;

        for (const ModInfo &mod : availableMods) {
            if (mod.id == config.modId) {
                found = true;
                result.availableModsCount++;
                if (!mod.nexusModId.isEmpty()) {
                    nexusId = mod.nexusModId;
                }
                break;
            }
        }

        if (!found) {
            MissingModInfo missingInfo;
            missingInfo.modId = config.modId;
            missingInfo.nexusModId = nexusId;
            result.missingMods.append(missingInfo);
        }
    }

    result.isValid = !result.hasMissingMods();
    return result;
}

bool ProfileManager::applyProfile(const QString &profileId, bool ignoreWarnings) {
    if (!m_modManager) {
        emit errorOccurred("ModManager not set");
        return false;
    }

    ProfileInfo profile = getProfile(profileId);
    if (profile.id.isEmpty()) {
        emit errorOccurred("Profile not found: " + profileId);
        return false;
    }

    if (!ignoreWarnings) {
        ProfileValidationResult validation = validateProfile(profileId);
        if (validation.hasMissingMods()) {
            emit errorOccurred("Cannot apply profile: some mods are missing. "
                             "Use validation to see details.");
            return false;
        }
    }

    bool success = applyProfileInternal(profile);
    if (success) {
        setActiveProfile(profileId);
        emit profileApplied(profileId);
        qDebug() << "Applied profile:" << profile.name;
    }

    return success;
}

bool ProfileManager::exportProfile(const QString &profileId, const QString &filePath) {
    ProfileInfo profile = getProfile(profileId);
    if (profile.id.isEmpty()) {
        emit errorOccurred("Profile not found: " + profileId);
        return false;
    }

    QJsonObject rootObj;
    rootObj["profile"] = profile.toJson();

    QJsonDocument doc(rootObj);
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Failed to open file for writing: " + filePath);
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Exported profile to:" << filePath;
    return true;
}

bool ProfileManager::importProfile(const QString &filePath, QString &importedProfileId) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Failed to open file for reading: " + filePath);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit errorOccurred("Invalid profile file format");
        return false;
    }

    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("profile")) {
        emit errorOccurred("Invalid profile file: missing 'profile' key");
        return false;
    }

    ProfileInfo profile = ProfileInfo::fromJson(rootObj["profile"].toObject());

    profile.id = ProfileInfo::generateId();

    QString baseName = profile.name;
    int suffix = 1;
    while (true) {
        bool nameExists = false;
        for (const ProfileInfo &existing : m_profiles) {
            if (existing.name == profile.name) {
                nameExists = true;
                break;
            }
        }

        if (!nameExists) break;

        profile.name = baseName + QString(" (%1)").arg(suffix);
        suffix++;
    }

    m_profiles.append(profile);
    importedProfileId = profile.id;

    saveProfiles();
    emit profileCreated(profile.id);
    emit profilesChanged();

    qDebug() << "Imported profile:" << profile.name;
    return true;
}

bool ProfileManager::loadProfiles() {
    QString filePath = getStorageFilePath();
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No profiles file found, starting fresh";
        return true;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit errorOccurred("Invalid profiles file format");
        return false;
    }

    QJsonObject rootObj = doc.object();
    QJsonArray profilesArray = rootObj["profiles"].toArray();

    m_profiles.clear();
    for (const QJsonValue &value : profilesArray) {
        ProfileInfo profile = ProfileInfo::fromJson(value.toObject());
        m_profiles.append(profile);
    }

    m_activeProfileId = rootObj["activeProfileId"].toString();

    qDebug() << "Loaded" << m_profiles.size() << "profiles";
    return true;
}

bool ProfileManager::saveProfiles() {
    QJsonArray profilesArray;
    for (const ProfileInfo &profile : m_profiles) {
        profilesArray.append(profile.toJson());
    }

    QJsonObject rootObj;
    rootObj["profiles"] = profilesArray;
    rootObj["activeProfileId"] = m_activeProfileId;

    QJsonDocument doc(rootObj);
    QString filePath = getStorageFilePath();
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Failed to save profiles");
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Saved" << m_profiles.size() << "profiles";
    return true;
}

void ProfileManager::setActiveProfile(const QString &profileId) {
    if (m_activeProfileId != profileId) {
        m_activeProfileId = profileId;
        saveProfiles();
        emit activeProfileChanged(profileId);
    }
}

void ProfileManager::clearActiveProfile() {
    if (!m_activeProfileId.isEmpty()) {
        m_activeProfileId.clear();
        saveProfiles();
        emit activeProfileChanged(QString());
    }
}

bool ProfileManager::isProfileActive(const QString &profileId) const {
    return m_activeProfileId == profileId;
}

QString ProfileManager::getStorageFilePath() const {
    return m_storagePath + "/profiles.json";
}

ProfileInfo ProfileManager::captureCurrentState() const {
    ProfileInfo profile;

    if (!m_modManager) {
        return profile;
    }

    QList<ModInfo> mods = m_modManager->getMods();
    for (const ModInfo &mod : mods) {
        ModConfig config;
        config.modId = mod.id;
        config.enabled = mod.enabled;
        config.priority = mod.priority;
        profile.modConfigs.append(config);
    }

    return profile;
}

bool ProfileManager::applyProfileInternal(const ProfileInfo &profile) {
    if (!m_modManager) {
        return false;
    }

    QList<ModInfo> allMods = m_modManager->getMods();
    for (const ModInfo &mod : allMods) {
        if (mod.enabled) {
            m_modManager->disableMod(mod.id);
        }
    }

    QMap<QString, int> priorityMap;
    for (const ModConfig &config : profile.modConfigs) {
        priorityMap.insert(config.modId, config.priority);
    }

    m_modManager->batchSetModPriorities(priorityMap);

    for (const ModConfig &config : profile.modConfigs) {
        if (config.enabled) {
            m_modManager->enableMod(config.modId);
        }
    }

    return true;
}
