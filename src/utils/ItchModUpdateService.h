#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QTimer>
#include "ItchUpdateInfo.h"
#include "ModInfo.h"

class ModManager;
class ItchClient;
struct ItchUploadInfo;

class ItchModUpdateService final : public QObject {
    Q_OBJECT

public:
    explicit ItchModUpdateService(ModManager *modManager,
                                 ItchClient *itchClient,
                                 QObject *parent = nullptr);
    ~ItchModUpdateService() override = default;

    bool hasUpdate(const QString &modId) const;
    ItchUpdateInfo getUpdateInfo(const QString &modId) const;

public slots:
    void checkAllModsForUpdates();
    void checkModForUpdate(const QString &modId);
    void cancelCheck();

signals:
    void checkStarted();
    void checkProgress(int current, int total);
    void updateFound(QString modId, ItchUpdateInfo updateInfo);
    void checkComplete(int updatesFound);
    void errorOccurred(QString message);

private slots:
    void onUploadsReceived(const QList<ItchUploadInfo> &uploads);
    void onError(const QString &error);

private:
    void processNextMod();
    bool isUpdateAvailable(const QDateTime &currentDate, const QDateTime &availableDate) const;
    void findLatestUpload(const QList<ItchUploadInfo> &uploads,
                         QString &latestVersion, QString &latestUploadId,
                         QDateTime &latestDate) const;
    QList<ItchUploadInfo> findCandidateUploads(const QList<ItchUploadInfo> &uploads,
                                               const QDateTime &currentDate) const;
    QString extractVersionFromFilename(const QString &filename) const;

    ModManager *m_modManager;
    ItchClient *m_itchClient;

    QMap<QString, ItchUpdateInfo> m_updateCache;
    QList<ModInfo> m_modsToCheck;
    int m_currentModIndex = 0;
    int m_totalMods = 0;
    int m_updatesFound = 0;
    bool m_isChecking = false;

    QTimer m_rateLimitTimer;
    static constexpr int RATE_LIMIT_DELAY_MS = 500;
};
