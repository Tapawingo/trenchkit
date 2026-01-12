#include "ModConflictDetector.h"
#include "ModManager.h"
#include "PakFileReader.h"
#include <QtConcurrent>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <algorithm>

ModConflictDetector::ModConflictDetector(ModManager *modManager, QObject *parent)
    : QObject(parent)
    , m_modManager(modManager)
    , m_scanWatcher(new QFutureWatcher<ScanResult>(this))
{
    connect(m_scanWatcher, &QFutureWatcher<ScanResult>::finished,
            this, &ModConflictDetector::onScanComplete);

    loadPersistentCache();
}

ModConflictDetector::~ModConflictDetector() {
    if (m_scanWatcher && m_scanWatcher->isRunning()) {
        m_scanWatcher->cancel();
        m_scanWatcher->waitForFinished();
    }
}

ConflictInfo ModConflictDetector::getConflictInfo(const QString &modId) const {
    QMutexLocker locker(&m_cacheMutex);
    return m_conflictCache.value(modId);
}

bool ModConflictDetector::hasConflicts(const QString &modId) const {
    QMutexLocker locker(&m_cacheMutex);
    return m_conflictCache.contains(modId) && m_conflictCache[modId].hasConflicts();
}

void ModConflictDetector::invalidateCache() {
    QMutexLocker locker(&m_cacheMutex);
    m_conflictCache.clear();
}

void ModConflictDetector::scanForConflicts() {
    if (m_isScanning) {
        return;
    }

    if (!m_modManager) {
        return;
    }

    ScanInput input;
    input.mods = m_modManager->getMods();
    input.modsStoragePath = m_modManager->getModsStoragePath();

    {
        QMutexLocker locker(&m_persistentCacheMutex);
        input.persistentCache = m_persistentCache;
    }

    m_isScanning = true;
    emit scanStarted();

    QFuture<ScanResult> future =
        QtConcurrent::run(&ModConflictDetector::detectConflictsWorker, input);

    m_scanWatcher->setFuture(future);
}

void ModConflictDetector::cancelScan() {
    if (m_scanWatcher && m_scanWatcher->isRunning()) {
        m_scanWatcher->cancel();
        emit scanCancelled();
    }
}

void ModConflictDetector::onScanComplete() {
    m_isScanning = false;

    if (m_scanWatcher->isCanceled()) {
        emit scanCancelled();
        return;
    }

    ScanResult result = m_scanWatcher->result();

    {
        QMutexLocker locker(&m_cacheMutex);
        m_conflictCache = result.conflicts;
    }

    {
        QMutexLocker locker(&m_persistentCacheMutex);
        m_persistentCache = result.updatedCache;
    }

    savePersistentCache();

    emit scanComplete(result.conflicts);
}

ModConflictDetector::ScanResult ModConflictDetector::detectConflictsWorker(const ScanInput &input)
{
    ScanResult result;
    QList<ModFileCache> modCaches;

    for (const ModInfo &mod : input.mods) {
        if (!mod.enabled) {
            continue;
        }

        QString pakPath = input.modsStoragePath + "/" + mod.fileName;
        QFileInfo fileInfo(pakPath);

        QStringList filePaths;
        bool usedCache = false;

        if (input.persistentCache.contains(mod.id)) {
            const PersistentModCache &cached = input.persistentCache[mod.id];

            if (cached.fileName == mod.fileName &&
                fileInfo.exists() &&
                cached.lastModified == fileInfo.lastModified() &&
                cached.fileSize == fileInfo.size()) {
                filePaths = cached.filePaths;
                usedCache = true;

                result.updatedCache[mod.id] = cached;
            }
        }

        if (!usedCache) {
            auto parseResult = PakFileReader::extractFilePaths(pakPath);

            if (parseResult.success) {
                filePaths = parseResult.filePaths;

                PersistentModCache newCache;
                newCache.fileName = mod.fileName;
                newCache.lastModified = fileInfo.lastModified();
                newCache.fileSize = fileInfo.size();
                newCache.filePaths = filePaths;
                result.updatedCache[mod.id] = newCache;
            } else {
                qDebug() << "Failed to parse" << mod.fileName << ":" << parseResult.error;
            }
        }

        if (!filePaths.isEmpty()) {
            ModFileCache cache;
            cache.modId = mod.id;
            cache.modName = mod.name;
            cache.priority = mod.priority;
            cache.filePaths = filePaths;
            modCaches.append(cache);
        }
    }

    QMap<QString, QList<const ModFileCache*>> fileToMods;
    for (const ModFileCache &cache : modCaches) {
        for (const QString &filePath : cache.filePaths) {
            fileToMods[filePath].append(&cache);
        }
    }

    for (auto it = fileToMods.constBegin(); it != fileToMods.constEnd(); ++it) {
        if (it.value().size() <= 1) {
            continue;
        }

        auto modList = it.value();
        std::sort(modList.begin(), modList.end(),
            [](const ModFileCache *a, const ModFileCache *b) {
                return a->priority < b->priority;
            });

        const QString &conflictingFile = it.key();
        const ModFileCache *winnerMod = modList.last();

        for (const ModFileCache *mod : modList) {
            ConflictInfo &info = result.conflicts[mod->modId];
            info.modPriority = mod->priority;
            info.fileConflictCount++;

            info.allConflictingFilePaths.append(conflictingFile);

            if (mod != winnerMod) {
                info.overwrittenFilePaths.insert(conflictingFile);
            }

            if (info.conflictingFilePaths.size() < 100) {
                info.conflictingFilePaths.append(conflictingFile);
            }

            for (const ModFileCache *otherMod : modList) {
                if (otherMod->modId != mod->modId) {
                    if (!info.conflictingModIds.contains(otherMod->modId)) {
                        info.conflictingModIds.append(otherMod->modId);
                        info.conflictingModNames.append(otherMod->modName);
                        info.conflictingModPriorities.append(otherMod->priority);
                    }
                }
            }
        }
    }

    return result;
}

QString ModConflictDetector::getCacheFilePath() const {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir appDataDir(appDataPath);
    if (!appDataDir.exists()) {
        appDataDir.mkpath(".");
    }
    return appDataPath + "/conflict_cache.json";
}

void ModConflictDetector::loadPersistentCache() {
    QMutexLocker locker(&m_persistentCacheMutex);

    QString cachePath = getCacheFilePath();
    QFile file(cachePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }

    QJsonObject root = doc.object();
    int version = root["version"].toInt();

    if (version != 1) {
        return;
    }

    QJsonObject modsObj = root["mods"].toObject();

    for (auto it = modsObj.constBegin(); it != modsObj.constEnd(); ++it) {
        QString modId = it.key();
        QJsonObject modObj = it.value().toObject();

        PersistentModCache cache;
        cache.fileName = modObj["fileName"].toString();
        cache.lastModified = QDateTime::fromString(modObj["lastModified"].toString(), Qt::ISODate);
        cache.fileSize = modObj["fileSize"].toVariant().toLongLong();

        QJsonArray filePathsArray = modObj["filePaths"].toArray();
        for (const QJsonValue &value : filePathsArray) {
            cache.filePaths.append(value.toString());
        }

        m_persistentCache[modId] = cache;
    }

    qDebug() << "Loaded persistent cache with" << m_persistentCache.size() << "mods";
}

void ModConflictDetector::savePersistentCache() {
    QMutexLocker locker(&m_persistentCacheMutex);

    QJsonObject root;
    root["version"] = 1;

    QJsonObject modsObj;

    for (auto it = m_persistentCache.constBegin(); it != m_persistentCache.constEnd(); ++it) {
        QString modId = it.key();
        const PersistentModCache &cache = it.value();

        QJsonObject modObj;
        modObj["fileName"] = cache.fileName;
        modObj["lastModified"] = cache.lastModified.toString(Qt::ISODate);
        modObj["fileSize"] = cache.fileSize;

        QJsonArray filePathsArray;
        for (const QString &filePath : cache.filePaths) {
            filePathsArray.append(filePath);
        }
        modObj["filePaths"] = filePathsArray;

        modsObj[modId] = modObj;
    }

    root["mods"] = modsObj;

    QJsonDocument doc(root);
    QString cachePath = getCacheFilePath();
    QFile file(cachePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to save persistent cache:" << file.errorString();
        return;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Saved persistent cache with" << m_persistentCache.size() << "mods";
}
