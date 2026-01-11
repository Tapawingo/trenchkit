#include "ModConflictDetector.h"
#include "ModManager.h"
#include "PakFileReader.h"
#include <QtConcurrent>
#include <QDebug>
#include <algorithm>

ModConflictDetector::ModConflictDetector(ModManager *modManager, QObject *parent)
    : QObject(parent)
    , m_modManager(modManager)
    , m_scanWatcher(new QFutureWatcher<QMap<QString, ConflictInfo>>(this))
{
    connect(m_scanWatcher, &QFutureWatcher<QMap<QString, ConflictInfo>>::finished,
            this, &ModConflictDetector::onScanComplete);
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

    QList<ModInfo> mods = m_modManager->getMods();
    QString storagePath = m_modManager->getModsStoragePath();

    m_isScanning = true;
    emit scanStarted();

    QFuture<QMap<QString, ConflictInfo>> future =
        QtConcurrent::run(&ModConflictDetector::detectConflictsWorker, mods, storagePath);

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

    QMap<QString, ConflictInfo> conflicts = m_scanWatcher->result();

    {
        QMutexLocker locker(&m_cacheMutex);
        m_conflictCache = conflicts;
    }

    emit scanComplete(conflicts);
}

QMap<QString, ConflictInfo> ModConflictDetector::detectConflictsWorker(
    const QList<ModInfo> &mods,
    const QString &modsStoragePath)
{
    QList<ModFileCache> modCaches;

    for (const ModInfo &mod : mods) {
        if (!mod.enabled) {
            continue;
        }

        QString pakPath = modsStoragePath + "/" + mod.fileName;
        auto result = PakFileReader::extractFilePaths(pakPath);

        if (result.success) {
            ModFileCache cache;
            cache.modId = mod.id;
            cache.modName = mod.name;
            cache.priority = mod.priority;
            cache.filePaths = result.filePaths;
            modCaches.append(cache);
        } else {
            qDebug() << "Failed to parse" << mod.fileName << ":" << result.error;
        }
    }

    QMap<QString, QList<const ModFileCache*>> fileToMods;
    for (const ModFileCache &cache : modCaches) {
        for (const QString &filePath : cache.filePaths) {
            fileToMods[filePath].append(&cache);
        }
    }

    QMap<QString, ConflictInfo> conflicts;

    for (auto it = fileToMods.constBegin(); it != fileToMods.constEnd(); ++it) {
        if (it.value().size() <= 1) {
            continue;
        }

        auto modList = it.value();
        std::sort(modList.begin(), modList.end(),
            [](const ModFileCache *a, const ModFileCache *b) {
                return a->priority < b->priority;
            });

        for (const ModFileCache *mod : modList) {
            ConflictInfo &info = conflicts[mod->modId];
            info.modPriority = mod->priority;
            info.fileConflictCount++;

            if (info.conflictingFilePaths.size() < 100) {
                info.conflictingFilePaths.append(it.key());
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

    return conflicts;
}
