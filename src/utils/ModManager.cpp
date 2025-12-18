#include "ModManager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>

ModManager::ModManager(QObject *parent)
    : QObject(parent)
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_modsStoragePath = appDataPath + "/mods";
    QDir().mkpath(m_modsStoragePath);
}

void ModManager::setInstallPath(const QString &foxholeInstallPath) {
    m_foxholeInstallPath = foxholeInstallPath;
}

void ModManager::setModsStoragePath(const QString &modsPath) {
    m_modsStoragePath = modsPath;
    QDir().mkpath(m_modsStoragePath);
}

QString ModManager::getPaksPath() const {
    if (m_foxholeInstallPath.isEmpty()) {
        return QString();
    }

    // Foxhole paks are typically in War/Content/Paks
    return m_foxholeInstallPath + "/War/Content/Paks";
}

bool ModManager::addMod(const QString &pakFilePath, const QString &modName) {
    QFileInfo fileInfo(pakFilePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        emit errorOccurred("Mod file does not exist: " + pakFilePath);
        return false;
    }

    ModInfo mod;
    mod.id = ModInfo::generateId();
    mod.fileName = fileInfo.fileName();
    mod.name = modName.isEmpty() ? cleanModName(fileInfo.fileName()) : modName;
    mod.installDate = QDateTime::currentDateTime();
    mod.enabled = false;
    mod.priority = m_mods.size();

    QString destPath = getModFilePath(mod.id);
    if (!QFile::copy(pakFilePath, destPath)) {
        emit errorOccurred("Failed to copy mod file to storage");
        return false;
    }

    m_mods.append(mod);
    saveMods();
    emit modAdded(mod.id);
    emit modsChanged();

    qDebug() << "Added mod:" << mod.name << "ID:" << mod.id;
    return true;
}

bool ModManager::removeMod(const QString &modId) {
    auto it = std::find_if(m_mods.begin(), m_mods.end(),
                           [&modId](const ModInfo &mod) { return mod.id == modId; });

    if (it == m_mods.end()) {
        emit errorOccurred("Mod not found: " + modId);
        return false;
    }

    if (it->enabled) {
        disableMod(modId);
    }

    QString filePath = getModFilePath(modId);
    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }

    m_mods.erase(it);
    saveMods();
    emit modRemoved(modId);
    emit modsChanged();

    qDebug() << "Removed mod:" << modId;
    return true;
}

bool ModManager::enableMod(const QString &modId) {
    auto it = std::find_if(m_mods.begin(), m_mods.end(),
                           [&modId](const ModInfo &mod) { return mod.id == modId; });

    if (it == m_mods.end()) {
        emit errorOccurred("Mod not found: " + modId);
        return false;
    }

    if (it->enabled) {
        return true;
    }

    if (!copyModToPaks(*it)) {
        emit errorOccurred("Failed to enable mod: " + it->name);
        return false;
    }

    it->enabled = true;
    saveMods();
    emit modEnabled(modId);
    emit modsChanged();

    qDebug() << "Enabled mod:" << it->name;
    return true;
}

bool ModManager::disableMod(const QString &modId) {
    auto it = std::find_if(m_mods.begin(), m_mods.end(),
                           [&modId](const ModInfo &mod) { return mod.id == modId; });

    if (it == m_mods.end()) {
        emit errorOccurred("Mod not found: " + modId);
        return false;
    }

    if (!it->enabled) {
        return true;
    }

    if (!removeModFromPaks(*it)) {
        emit errorOccurred("Failed to disable mod: " + it->name);
        return false;
    }

    it->enabled = false;
    saveMods();
    emit modDisabled(modId);
    emit modsChanged();

    qDebug() << "Disabled mod:" << it->name;
    return true;
}

bool ModManager::setModPriority(const QString &modId, int priority) {
    auto it = std::find_if(m_mods.begin(), m_mods.end(),
                           [&modId](const ModInfo &mod) { return mod.id == modId; });

    if (it == m_mods.end()) {
        return false;
    }

    it->priority = priority;
    sortModsByPriority();

    // Renumber all enabled mods to reflect new priority order
    renumberEnabledMods();

    saveMods();
    emit modsChanged();

    return true;
}

QList<ModInfo> ModManager::getMods() const {
    return m_mods;
}

ModInfo ModManager::getMod(const QString &modId) const {
    auto it = std::find_if(m_mods.begin(), m_mods.end(),
                           [&modId](const ModInfo &mod) { return mod.id == modId; });
    return it != m_mods.end() ? *it : ModInfo();
}

bool ModManager::loadMods() {
    QString metadataPath = getMetadataFilePath();
    QFile file(metadataPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No mods metadata file found, starting fresh";
        return true;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        emit errorOccurred("Invalid mods metadata format");
        return false;
    }

    m_mods.clear();
    QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        ModInfo mod = ModInfo::fromJson(value.toObject());
        m_mods.append(mod);
    }

    sortModsByPriority();
    qDebug() << "Loaded" << m_mods.size() << "mods";
    return true;
}

bool ModManager::saveMods() {
    QJsonArray array;
    for (const ModInfo &mod : m_mods) {
        array.append(mod.toJson());
    }

    QJsonDocument doc(array);
    QString metadataPath = getMetadataFilePath();
    QFile file(metadataPath);

    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Failed to save mods metadata");
        return false;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Saved" << m_mods.size() << "mods";
    return true;
}

QString ModManager::getModFilePath(const QString &modId) const {
    auto it = std::find_if(m_mods.begin(), m_mods.end(),
                           [&modId](const ModInfo &mod) { return mod.id == modId; });

    if (it != m_mods.end()) {
        return m_modsStoragePath + "/" + it->fileName;
    }
    return QString();
}

QString ModManager::getMetadataFilePath() const {
    return m_modsStoragePath + "/mods.json";
}

bool ModManager::copyModToPaks(const ModInfo &mod) {
    QString paksPath = getPaksPath();
    if (paksPath.isEmpty()) {
        return false;
    }

    QDir paksDir(paksPath);
    if (!paksDir.exists()) {
        emit errorOccurred("Paks directory not found: " + paksPath);
        return false;
    }

    QString sourcePath = m_modsStoragePath + "/" + mod.fileName;
    if (!QFile::exists(sourcePath)) {
        emit errorOccurred("Mod file not found in storage: " + sourcePath);
        return false;
    }

    if (!mod.numberedFileName.isEmpty()) {
        QString oldPath = paksPath + "/" + mod.numberedFileName;
        if (QFile::exists(oldPath)) {
            QFile::remove(oldPath);
        }
    }

    QString numberedName = generateNumberedFileName(mod.priority, mod.fileName);
    QString destPath = paksPath + "/" + numberedName;

    if (!QFile::copy(sourcePath, destPath)) {
        return false;
    }

    auto it = std::find_if(m_mods.begin(), m_mods.end(),
                           [&mod](const ModInfo &m) { return m.id == mod.id; });
    if (it != m_mods.end()) {
        it->numberedFileName = numberedName;
    }

    qDebug() << "Copied mod to paks:" << numberedName;
    return true;
}

bool ModManager::removeModFromPaks(const ModInfo &mod) {
    QString paksPath = getPaksPath();
    if (paksPath.isEmpty()) {
        return false;
    }

    if (!mod.numberedFileName.isEmpty()) {
        QString filePath = paksPath + "/" + mod.numberedFileName;
        if (QFile::exists(filePath)) {
            bool removed = QFile::remove(filePath);
            if (removed) {
                qDebug() << "Removed mod from paks:" << mod.numberedFileName;
            }
            return removed;
        }
    }

    QString filePath = paksPath + "/" + mod.fileName;
    if (QFile::exists(filePath)) {
        bool removed = QFile::remove(filePath);
        if (removed) {
            qDebug() << "Removed mod from paks:" << mod.fileName;
        }
        return removed;
    }

    return true;
}

void ModManager::sortModsByPriority() {
    std::sort(m_mods.begin(), m_mods.end(),
              [](const ModInfo &a, const ModInfo &b) {
                  return a.priority < b.priority;
              });
}

QString ModManager::generateNumberedFileName(int priority, const QString &originalFileName) const {
    // Generate a 3-digit prefix (e.g., "001_", "002_", etc.)
    QString prefix = QString("%1_").arg(priority, 3, 10, QChar('0'));
    return prefix + originalFileName;
}

void ModManager::updateNumberedFileNames() {
    for (int i = 0; i < m_mods.size(); ++i) {
        m_mods[i].numberedFileName = generateNumberedFileName(i, m_mods[i].fileName);
    }
}

void ModManager::renumberEnabledMods() {
    QString paksPath = getPaksPath();
    if (paksPath.isEmpty()) {
        return;
    }

    for (auto &mod : m_mods) {
        if (!mod.enabled) {
            continue;
        }

        QString oldNumberedName = mod.numberedFileName;
        QString newNumberedName = generateNumberedFileName(mod.priority, mod.fileName);

        if (oldNumberedName == newNumberedName) {
            continue;
        }

        QString oldPath = paksPath + "/" + oldNumberedName;
        QString newPath = paksPath + "/" + newNumberedName;

        if (QFile::exists(oldPath)) {
            QFile::remove(oldPath);
        }

        QString sourcePath = m_modsStoragePath + "/" + mod.fileName;
        if (QFile::exists(sourcePath)) {
            QFile::copy(sourcePath, newPath);
            mod.numberedFileName = newNumberedName;
            qDebug() << "Renumbered mod:" << oldNumberedName << "->" << newNumberedName;
        }
    }
}

void ModManager::detectUnregisteredMods() {
    QString paksPath = getPaksPath();
    if (paksPath.isEmpty()) {
        qDebug() << "Cannot detect unregistered mods: paks path not set";
        return;
    }

    QDir paksDir(paksPath);
    if (!paksDir.exists()) {
        qDebug() << "Paks directory does not exist:" << paksPath;
        return;
    }

    QStringList pakFiles = paksDir.entryList(QStringList() << "*.pak", QDir::Files);

    for (const QString &pakFile : pakFiles) {
        if (isBaseGamePak(pakFile)) {
            qDebug() << "Skipping base game pak:" << pakFile;
            continue;
        }

        bool isRegistered = false;
        QString originalFileName = pakFile;

        // Strip numeric prefix (e.g., "001_modname.pak" -> "modname.pak")
        QRegularExpression numPrefixRegex(R"(^\d{3}_(.+)$)");
        QRegularExpressionMatch match = numPrefixRegex.match(pakFile);
        if (match.hasMatch()) {
            originalFileName = match.captured(1);
        }

        for (const ModInfo &mod : m_mods) {
            if (mod.fileName == originalFileName || mod.numberedFileName == pakFile) {
                isRegistered = true;
                break;
            }
        }

        if (!isRegistered) {
            QString pakFilePath = paksPath + "/" + pakFile;

            ModInfo mod;
            mod.id = ModInfo::generateId();
            mod.fileName = originalFileName;
            mod.numberedFileName = pakFile;
            mod.name = cleanModName(originalFileName);
            mod.installDate = QFileInfo(pakFilePath).lastModified();
            mod.enabled = true;
            mod.priority = m_mods.size();

            QString destPath = m_modsStoragePath + "/" + originalFileName;
            if (!QFile::exists(destPath)) {
                if (QFile::copy(pakFilePath, destPath)) {
                    m_mods.append(mod);
                    qDebug() << "Detected and registered unregistered mod:" << originalFileName;
                }
            } else {
                m_mods.append(mod);
                qDebug() << "Registered existing mod from paks:" << originalFileName;
            }
        }
    }

    if (!pakFiles.isEmpty()) {
        sortModsByPriority();
        saveMods();
        emit modsChanged();
    }
}

void ModManager::syncEnabledModsWithPaks() {
    // This ensures all enabled mods are properly numbered in the paks folder
    updateNumberedFileNames();
    renumberEnabledMods();
    saveMods();
}

QString ModManager::cleanModName(const QString &fileName) const {
    QString baseName = QFileInfo(fileName).baseName();

    if (baseName.startsWith("War-WindowsNoEditor", Qt::CaseInsensitive)) {
        baseName = baseName.mid(19);
    }

    if (baseName.endsWith("-WindowsNoEditor", Qt::CaseInsensitive)) {
        baseName.chop(16);
    }

    baseName.replace("WindowsNoEditor", "", Qt::CaseInsensitive);

    while (baseName.startsWith('-') || baseName.startsWith('_')) {
        baseName = baseName.mid(1);
    }
    while (baseName.endsWith('-') || baseName.endsWith('_')) {
        baseName.chop(1);
    }

    baseName.replace('_', ' ');

    return baseName.isEmpty() ? fileName : baseName;
}

bool ModManager::isBaseGamePak(const QString &fileName) const {
    QString lowerFileName = fileName.toLower();

    QRegularExpression pakChunkRegex(R"(^pakchunk\d+.*\.pak$)", QRegularExpression::CaseInsensitiveOption);
    if (pakChunkRegex.match(lowerFileName).hasMatch()) {
        return true;
    }

    if (lowerFileName == "war-windowsnoeditor.pak" || lowerFileName == "war.pak") {
        return true;
    }

    return false;
}
