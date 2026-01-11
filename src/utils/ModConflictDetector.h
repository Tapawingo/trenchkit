#ifndef MODCONFLICTDETECTOR_H
#define MODCONFLICTDETECTOR_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QStringList>
#include <QFutureWatcher>
#include <QMutex>
#include "ModInfo.h"

struct ConflictInfo {
    int modPriority = 0;
    QStringList conflictingModIds;
    QStringList conflictingModNames;
    QList<int> conflictingModPriorities;
    QStringList conflictingFilePaths;
    int fileConflictCount = 0;

    bool hasConflicts() const { return !conflictingModIds.isEmpty(); }
};

class ModManager;

class ModConflictDetector : public QObject {
    Q_OBJECT

public:
    explicit ModConflictDetector(ModManager *modManager, QObject *parent = nullptr);
    ~ModConflictDetector() override;

    ConflictInfo getConflictInfo(const QString &modId) const;
    bool hasConflicts(const QString &modId) const;
    void invalidateCache();

public slots:
    void scanForConflicts();
    void cancelScan();

signals:
    void scanStarted();
    void scanProgress(int current, int total);
    void scanComplete(QMap<QString, ConflictInfo> conflicts);
    void scanCancelled();
    void errorOccurred(QString message);

private slots:
    void onScanComplete();

private:
    struct ModFileCache {
        QString modId;
        QString modName;
        int priority;
        QStringList filePaths;
    };

    static QMap<QString, ConflictInfo> detectConflictsWorker(
        const QList<ModInfo> &mods,
        const QString &modsStoragePath
    );

    ModManager *m_modManager;
    QFutureWatcher<QMap<QString, ConflictInfo>> *m_scanWatcher;
    QMap<QString, ConflictInfo> m_conflictCache;
    mutable QMutex m_cacheMutex;
    bool m_isScanning = false;
};

#endif // MODCONFLICTDETECTOR_H
