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

bool ModManager::addMod(const QString &pakFilePath, const QString &modName,
                        const QString &nexusModId, const QString &nexusFileId,
                        const QString &author, const QString &description,
                        const QString &version, const QString &itchGameId,
                        const QDateTime &uploadDate) {
    QFileInfo fileInfo(pakFilePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        emit errorOccurred("Mod file does not exist: " + pakFilePath);
        return false;
    }

    ModInfo mod;
    mod.id = ModInfo::generateId();

    if (!modName.isEmpty()) {
        mod.fileName = modName + fileInfo.suffix().prepend('.');
        mod.name = modName;
    } else {
        mod.fileName = fileInfo.fileName();
        mod.name = cleanModName(fileInfo.fileName());
    }

    mod.installDate = QDateTime::currentDateTime();
    mod.uploadDate = uploadDate;
    mod.enabled = false;

    {
        QMutexLocker locker(&m_modsMutex);
        mod.priority = m_mods.size();
    }

    mod.nexusModId = nexusModId;
    mod.nexusFileId = nexusFileId;
    mod.itchGameId = itchGameId;
    mod.author = author;
    mod.description = description;
    mod.version = version;

    QString destPath = m_modsStoragePath + "/" + mod.fileName;
    if (!QFile::copy(pakFilePath, destPath)) {
        emit errorOccurred("Failed to copy mod file to storage");
        return false;
    }

    {
        QMutexLocker locker(&m_modsMutex);
        m_mods.append(mod);
    }
    saveMods();
    emit modAdded(mod.id);
    emit modsChanged();

    qDebug() << "Added mod:" << mod.name << "ID:" << mod.id;
    return true;
}

bool ModManager::removeMod(const QString &modId) {
    bool wasEnabled;
    QString filePath;

    {
        QMutexLocker locker(&m_modsMutex);
        auto it = std::find_if(m_mods.begin(), m_mods.end(),
                               [&modId](const ModInfo &mod) { return mod.id == modId; });

        if (it == m_mods.end()) {
            emit errorOccurred("Mod not found: " + modId);
            return false;
        }

        wasEnabled = it->enabled;
        filePath = getModFilePath(modId);
    }

    if (wasEnabled) {
        disableMod(modId);
    }

    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }

    {
        QMutexLocker locker(&m_modsMutex);
        auto it = std::find_if(m_mods.begin(), m_mods.end(),
                               [&modId](const ModInfo &mod) { return mod.id == modId; });
        if (it != m_mods.end()) {
            m_mods.erase(it);
        }
    }

    saveMods();
    emit modRemoved(modId);
    emit modsChanged();

    qDebug() << "Removed mod:" << modId;
    return true;
}

bool ModManager::replaceMod(const QString &modId, const QString &newPakPath,
                            const QString &newVersion, const QString &newFileId,
                            const QDateTime &uploadDate) {
    bool wasEnabled;
    int savedPriority;
    QString savedName;
    QString fileName;

    {
        QMutexLocker locker(&m_modsMutex);
        auto it = std::find_if(m_mods.begin(), m_mods.end(),
                               [&modId](const ModInfo &mod) { return mod.id == modId; });

        if (it == m_mods.end()) {
            emit errorOccurred("Mod not found: " + modId);
            return false;
        }

        QFileInfo newFileInfo(newPakPath);
        if (!newFileInfo.exists() || !newFileInfo.isFile()) {
            emit errorOccurred("New mod file does not exist: " + newPakPath);
            return false;
        }

        wasEnabled = it->enabled;
        savedPriority = it->priority;
        savedName = it->name;
        fileName = it->fileName;
    }

    if (wasEnabled) {
        if (!disableMod(modId)) {
            emit errorOccurred("Failed to disable mod before replacement");
            return false;
        }
    }

    QString oldPath = m_modsStoragePath + "/" + fileName;
    if (QFile::exists(oldPath)) {
        if (!QFile::remove(oldPath)) {
            emit errorOccurred("Failed to remove old mod file");
            if (wasEnabled) {
                QMutexLocker locker(&m_modsMutex);
                auto it = std::find_if(m_mods.begin(), m_mods.end(),
                                       [&modId](const ModInfo &mod) { return mod.id == modId; });
                if (it != m_mods.end()) {
                    it->priority = savedPriority;
                }
                locker.unlock();
                enableMod(modId);
            }
            return false;
        }
    }

    QString destPath = m_modsStoragePath + "/" + fileName;
    if (!QFile::copy(newPakPath, destPath)) {
        emit errorOccurred("Failed to copy new mod file to storage");
        return false;
    }

    {
        QMutexLocker locker(&m_modsMutex);
        auto it = std::find_if(m_mods.begin(), m_mods.end(),
                               [&modId](const ModInfo &mod) { return mod.id == modId; });
        if (it != m_mods.end()) {
            it->version = newVersion;
            it->nexusFileId = newFileId;
            it->uploadDate = uploadDate;
            it->installDate = QDateTime::currentDateTime();

            if (wasEnabled) {
                it->priority = savedPriority;
            }
        }
    }

    if (wasEnabled) {
        if (!enableMod(modId)) {
            emit errorOccurred("Failed to re-enable mod after replacement");
        }
    }

    saveMods();
    emit modsChanged();

    qDebug() << "Replaced mod:" << savedName << "version" << newVersion;
    return true;
}

bool ModManager::enableMod(const QString &modId) {
    ModInfo modCopy;
    bool alreadyEnabled = false;

    {
        QMutexLocker locker(&m_modsMutex);
        auto it = std::find_if(m_mods.begin(), m_mods.end(),
                               [&modId](const ModInfo &mod) { return mod.id == modId; });

        if (it == m_mods.end()) {
            emit errorOccurred("Mod not found: " + modId);
            return false;
        }

        if (it->enabled) {
            alreadyEnabled = true;
        } else {
            modCopy = *it;
        }
    }

    if (alreadyEnabled) {
        return true;
    }

    if (!copyModToPaks(modCopy)) {
        emit errorOccurred("Failed to enable mod: " + modCopy.name);
        return false;
    }

    {
        QMutexLocker locker(&m_modsMutex);
        auto it = std::find_if(m_mods.begin(), m_mods.end(),
                               [&modId](const ModInfo &mod) { return mod.id == modId; });
        if (it != m_mods.end()) {
            it->enabled = true;
        }
    }

    saveMods();
    emit modEnabled(modId);
    emit modsChanged();

    qDebug() << "Enabled mod:" << modCopy.name;
    return true;
}

bool ModManager::disableMod(const QString &modId) {
    ModInfo modCopy;
    bool alreadyDisabled = false;

    {
        QMutexLocker locker(&m_modsMutex);
        auto it = std::find_if(m_mods.begin(), m_mods.end(),
                               [&modId](const ModInfo &mod) { return mod.id == modId; });

        if (it == m_mods.end()) {
            emit errorOccurred("Mod not found: " + modId);
            return false;
        }

        if (!it->enabled) {
            alreadyDisabled = true;
        } else {
            modCopy = *it;
        }
    }

    if (alreadyDisabled) {
        return true;
    }

    if (!removeModFromPaks(modCopy)) {
        emit errorOccurred("Failed to disable mod: " + modCopy.name);
        return false;
    }

    {
        QMutexLocker locker(&m_modsMutex);
        auto it = std::find_if(m_mods.begin(), m_mods.end(),
                               [&modId](const ModInfo &mod) { return mod.id == modId; });
        if (it != m_mods.end()) {
            it->enabled = false;
        }
    }

    saveMods();
    emit modDisabled(modId);
    emit modsChanged();

    qDebug() << "Disabled mod:" << modCopy.name;
    return true;
}

bool ModManager::setModPriority(const QString &modId, int priority) {
    QMutexLocker locker(&m_modsMutex);
    auto it = std::find_if(m_mods.begin(), m_mods.end(),
                           [&modId](const ModInfo &mod) { return mod.id == modId; });

    if (it == m_mods.end()) {
        return false;
    }

    it->priority = priority;
    sortModsByPriority();

    // Renumber all enabled mods to reflect new priority order
    renumberEnabledMods();
    locker.unlock();

    saveMods();
    emit modsChanged();

    return true;
}

bool ModManager::batchSetModPriorities(const QMap<QString, int> &priorityMap) {
    if (priorityMap.isEmpty()) {
        return false;
    }

    bool wasBlocked = blockSignals(true);

    QMutexLocker locker(&m_modsMutex);
    bool anyChanged = false;
    for (auto it = priorityMap.constBegin(); it != priorityMap.constEnd(); ++it) {
        const QString &modId = it.key();
        int newPriority = it.value();

        auto modIt = std::find_if(m_mods.begin(), m_mods.end(),
                                   [&modId](const ModInfo &mod) { return mod.id == modId; });

        if (modIt != m_mods.end() && modIt->priority != newPriority) {
            modIt->priority = newPriority;
            anyChanged = true;
        }
    }

    if (anyChanged) {
        sortModsByPriority();
        renumberEnabledMods();
    }
    locker.unlock();

    if (anyChanged) {
        saveMods();
    }

    blockSignals(wasBlocked);

    if (anyChanged) {
        emit modsChanged();
    }

    return anyChanged;
}

bool ModManager::updateModMetadata(const ModInfo &updatedMod) {
    QMutexLocker locker(&m_modsMutex);
    auto it = std::find_if(m_mods.begin(), m_mods.end(),
                           [&updatedMod](const ModInfo &mod) { return mod.id == updatedMod.id; });

    if (it == m_mods.end()) {
        emit errorOccurred("Mod not found: " + updatedMod.id);
        return false;
    }

    QString oldFileName = it->fileName;
    bool fileNameChanged = (oldFileName != updatedMod.fileName);

    it->name = updatedMod.name;
    it->description = updatedMod.description;
    it->nexusModId = updatedMod.nexusModId;
    it->nexusFileId = updatedMod.nexusFileId;
    it->itchGameId = updatedMod.itchGameId;
    it->version = updatedMod.version;
    it->author = updatedMod.author;
    it->installDate = updatedMod.installDate;
    it->uploadDate = updatedMod.uploadDate;
    it->fileName = updatedMod.fileName;
    it->ignoredItchUploadIds = updatedMod.ignoredItchUploadIds;

    if (fileNameChanged) {
        QString oldPath = m_modsStoragePath + "/" + oldFileName;
        QString newPath = m_modsStoragePath + "/" + updatedMod.fileName;
        locker.unlock();

        if (QFile::exists(oldPath) && oldPath != newPath) {
            QFile::rename(oldPath, newPath);
        }

        locker.relock();
        auto it2 = std::find_if(m_mods.begin(), m_mods.end(),
                               [&updatedMod](const ModInfo &mod) { return mod.id == updatedMod.id; });
        if (it2 != m_mods.end() && it2->enabled) {
            it2->numberedFileName = generateNumberedFileName(it2->priority, updatedMod.fileName);
            renumberEnabledMods();
        }
    }
    locker.unlock();

    saveMods();
    emit modsChanged();

    qDebug() << "Updated mod metadata:" << updatedMod.name;
    return true;
}

QList<ModInfo> ModManager::getMods() const {
    QMutexLocker locker(&m_modsMutex);
    return m_mods;
}

ModInfo ModManager::getMod(const QString &modId) const {
    QMutexLocker locker(&m_modsMutex);
    auto it = std::find_if(m_mods.begin(), m_mods.end(),
                           [&modId](const ModInfo &mod) { return mod.id == modId; });
    return it != m_mods.end() ? *it : ModInfo();
}

bool ModManager::loadMods() {
    qDebug() << "=== loadMods START";

    QString metadataPath = getMetadataFilePath();
    QFile file(metadataPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "No mods metadata file found, starting fresh";
        qDebug() << "=== loadMods END, loaded: 0 mods";
        return true;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        emit errorOccurred("Invalid mods metadata format");
        return false;
    }

    QMutexLocker locker(&m_modsMutex);
    m_mods.clear();
    QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        ModInfo mod = ModInfo::fromJson(value.toObject());
        m_mods.append(mod);
    }

    sortModsByPriority();
    qDebug() << "=== loadMods END, loaded:" << m_mods.size() << "mods";
    return true;
}

bool ModManager::saveMods() {
    QMutexLocker locker(&m_modsMutex);
    QJsonArray array;
    for (const ModInfo &mod : m_mods) {
        array.append(mod.toJson());
    }
    locker.unlock();

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
    QMutexLocker locker(&m_modsMutex);
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

    {
        QMutexLocker locker(&m_modsMutex);
        auto it = std::find_if(m_mods.begin(), m_mods.end(),
                               [&mod](const ModInfo &m) { return m.id == mod.id; });
        if (it != m_mods.end()) {
            it->numberedFileName = numberedName;
        }
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
    QMutexLocker locker(&m_modsMutex);
    std::sort(m_mods.begin(), m_mods.end(),
              [](const ModInfo &a, const ModInfo &b) {
                  return a.priority < b.priority;
              });
}

QString ModManager::generateNumberedFileName(int priority, const QString &originalFileName) const {
    QString number = QString("%1").arg(priority, 3, 10, QChar('0'));

    if (originalFileName.startsWith("War-WindowsNoEditor", Qt::CaseInsensitive)) {
        QString remaining = originalFileName.mid(19);
        if (remaining.startsWith("_") || remaining.startsWith("-")) {
            return "War-WindowsNoEditor_" + number + remaining;
        } else {
            return "War-WindowsNoEditor_" + number + "_" + remaining;
        }
    } else {
        return number + "_" + originalFileName;
    }
}

void ModManager::updateNumberedFileNames() {
    QMutexLocker locker(&m_modsMutex);
    for (int i = 0; i < m_mods.size(); ++i) {
        m_mods[i].numberedFileName = generateNumberedFileName(i, m_mods[i].fileName);
    }
}

void ModManager::renumberEnabledMods() {
    QString paksPath = getPaksPath();
    if (paksPath.isEmpty()) {
        return;
    }

    QMutexLocker locker(&m_modsMutex);
    for (auto &mod : m_mods) {
        if (!mod.enabled) {
            continue;
        }

        QString oldNumberedName = mod.numberedFileName;
        QString newNumberedName = generateNumberedFileName(mod.priority, mod.fileName);

        if (oldNumberedName == newNumberedName) {
            QString expectedPath = paksPath + "/" + newNumberedName;
            if (QFile::exists(expectedPath)) {
                continue;
            }

            QString sourcePath = m_modsStoragePath + "/" + mod.fileName;
            locker.unlock();

            if (QFile::exists(sourcePath)) {
                QFile::copy(sourcePath, expectedPath);
                locker.relock();
                mod.numberedFileName = newNumberedName;
                qDebug() << "Restored mod to paks:" << newNumberedName;
                locker.unlock();
            } else {
                locker.relock();
            }
            continue;
        }

        QString oldPath = paksPath + "/" + oldNumberedName;
        QString newPath = paksPath + "/" + newNumberedName;
        locker.unlock();

        if (QFile::exists(oldPath)) {
            QFile::remove(oldPath);
        }

        QString sourcePath = m_modsStoragePath + "/" + mod.fileName;
        if (QFile::exists(sourcePath)) {
            QFile::copy(sourcePath, newPath);
            locker.relock();
            mod.numberedFileName = newNumberedName;
            qDebug() << "Renumbered mod:" << oldNumberedName << "->" << newNumberedName;
            locker.unlock();
        } else {
            locker.relock();
        }
    }
}

void ModManager::detectUnregisteredMods() {
    qDebug() << "=== detectUnregisteredMods START, current size:" << m_mods.size();

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

        QRegularExpression warPrefixRegex(R"(^War-WindowsNoEditor_\d{3}_(.+)$)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch warMatch = warPrefixRegex.match(pakFile);
        if (warMatch.hasMatch()) {
            originalFileName = "War-WindowsNoEditor_" + warMatch.captured(1);
        } else {
            QRegularExpression numPrefixRegex(R"(^\d{3}_(.+)$)");
            QRegularExpressionMatch numMatch = numPrefixRegex.match(pakFile);
            if (numMatch.hasMatch()) {
                originalFileName = numMatch.captured(1);
            }
        }

        // Check if already registered (case-insensitive)
        {
            QMutexLocker locker(&m_modsMutex);
            for (const ModInfo &mod : m_mods) {
                if (mod.fileName.compare(originalFileName, Qt::CaseInsensitive) == 0 ||
                    mod.numberedFileName.compare(pakFile, Qt::CaseInsensitive) == 0) {
                    isRegistered = true;
                    break;
                }
            }
        }

        if (!isRegistered) {
            QString pakFilePath = paksPath + "/" + pakFile;

            ModInfo newMod;
            newMod.id = ModInfo::generateId();
            newMod.fileName = originalFileName;
            newMod.numberedFileName = pakFile;
            newMod.name = cleanModName(originalFileName);
            newMod.installDate = QFileInfo(pakFilePath).lastModified();
            newMod.enabled = true;

            QString destPath = m_modsStoragePath + "/" + originalFileName;

            // Double-check registration status (mod may have been loaded between initial check and now)
            bool stillUnregistered = true;
            {
                QMutexLocker locker(&m_modsMutex);
                for (const ModInfo &existingMod : m_mods) {
                    if (existingMod.fileName.compare(originalFileName, Qt::CaseInsensitive) == 0 ||
                        existingMod.numberedFileName.compare(pakFile, Qt::CaseInsensitive) == 0) {
                        stillUnregistered = false;
                        break;
                    }
                }

                if (!stillUnregistered) {
                    qDebug() << "Mod already registered (caught in double-check):" << originalFileName;
                    continue;
                }

                newMod.priority = m_mods.size();
            }

            // Copy file if needed (outside lock for I/O)
            bool needsCopy = !QFile::exists(destPath);
            if (needsCopy) {
                if (!QFile::copy(pakFilePath, destPath)) {
                    qDebug() << "Failed to copy mod file:" << originalFileName;
                    continue;
                }
                qDebug() << "Detected and registered unregistered mod:" << originalFileName;
            } else {
                qDebug() << "Registered existing mod from paks:" << originalFileName;
            }

            // Add to list with lock
            {
                QMutexLocker locker(&m_modsMutex);
                m_mods.append(newMod);
            }
        }
    }

    if (!pakFiles.isEmpty()) {
        // Deduplication safety net: remove any duplicates that might have slipped through
        {
            QMutexLocker locker(&m_modsMutex);

            QMap<QString, int> seenMods;  // fileName.toLower() -> first index
            QList<int> indicesToRemove;

            for (int i = 0; i < m_mods.size(); ++i) {
                QString lowerFileName = m_mods[i].fileName.toLower();
                if (seenMods.contains(lowerFileName)) {
                    int originalIndex = seenMods[lowerFileName];
                    qWarning() << "Duplicate mod detected:" << m_mods[i].fileName
                              << "(keeping first at index" << originalIndex << ", removing duplicate at" << i << ")";
                    indicesToRemove.append(i);
                } else {
                    seenMods[lowerFileName] = i;
                }
            }

            // Remove duplicates in reverse order to maintain indices
            std::sort(indicesToRemove.begin(), indicesToRemove.end(), std::greater<int>());
            for (int idx : indicesToRemove) {
                m_mods.removeAt(idx);
            }

            if (!indicesToRemove.isEmpty()) {
                qWarning() << "Removed" << indicesToRemove.size() << "duplicate mod(s)";
            }
        }

        sortModsByPriority();
        saveMods();
        emit modsChanged();
    }

    qDebug() << "=== detectUnregisteredMods END, final size:" << m_mods.size();
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
