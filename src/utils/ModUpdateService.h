#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QTimer>
#include "ModUpdateInfo.h"
#include "ModInfo.h"

class ModManager;
class NexusModsClient;
struct NexusFileInfo;

class ModUpdateService final : public QObject {
    Q_OBJECT

public:
    explicit ModUpdateService(ModManager *modManager,
                             NexusModsClient *nexusClient,
                             QObject *parent = nullptr);
    ~ModUpdateService() override = default;

    bool hasUpdate(const QString &modId) const;
    ModUpdateInfo getUpdateInfo(const QString &modId) const;

public slots:
    void checkAllModsForUpdates();
    void checkModForUpdate(const QString &modId);
    void cancelCheck();

signals:
    void checkStarted();
    void checkProgress(int current, int total);
    void updateFound(QString modId, ModUpdateInfo updateInfo);
    void checkComplete(int updatesFound);
    void errorOccurred(QString message);

private slots:
    void onModFilesReceived(const QList<NexusFileInfo> &files);
    void onError(const QString &error);

private:
    void processNextMod();
    bool isUpdateAvailable(const QString &currentVersion, const QString &availableVersion) const;
    void findLatestVersion(const QList<NexusFileInfo> &files, const QString &currentFileId,
                          QString &latestVersion, QString &latestFileId, QDateTime &latestDate,
                          bool &isExplicit) const;

    ModManager *m_modManager;
    NexusModsClient *m_nexusClient;

    QMap<QString, ModUpdateInfo> m_updateCache;
    QList<ModInfo> m_modsToCheck;
    int m_currentModIndex = 0;
    int m_totalMods = 0;
    int m_updatesFound = 0;
    bool m_isChecking = false;

    QTimer m_rateLimitTimer;
    static constexpr int RATE_LIMIT_DELAY_MS = 500;
};
