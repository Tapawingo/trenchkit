#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QVersionNumber>
#include <QPointer>
#include <QFile>
#include <QDateTime>

class UpdaterService final : public QObject {
    Q_OBJECT
public:
    struct Asset {
        QString name;
        QUrl downloadUrl;
        qint64 sizeBytes = -1;
        QString contentType;
    };

    struct ReleaseInfo {
        QString tagName;
        QString name;
        QString body;
        QUrl htmlUrl;
        QDateTime publishedAt;
        QList<Asset> assets;
        bool prerelease = false;
        bool draft = false;

        QVersionNumber version;
    };

    explicit UpdaterService(QString owner,
                            QString repo,
                            QObject* parent = nullptr);

    void setAuthToken(const QString& token);
    void setRepository(const QString& owner, const QString& repo);
    QString owner() const { return m_owner; }
    QString repo() const { return m_repo; }
    void setIncludePrereleases(bool include);
    bool includePrereleases() const { return m_includePrereleases; }

    QVersionNumber currentVersion() const;
    static QVersionNumber parseVersionFromTag(const QString& tag);

public slots:
    void checkForUpdates();
    void downloadAsset(const Asset& asset, const QString& savePath);
    void cancelDownload();

signals:
    void checkingStarted();
    void updateAvailable(UpdaterService::ReleaseInfo release);
    void upToDate(UpdaterService::ReleaseInfo latest);
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(QString savePath);
    void errorOccurred(QString message);

private:
    QNetworkRequest makeRequest(const QUrl& url) const;
    void handleReleaseJson(const QByteArray& json);
    void startDownload(bool allowResume, bool forceRestart);
    void restartDownloadFromScratch();
    QString formatNetworkError(const QString& context,
                               QNetworkReply* reply,
                               const QByteArray& body) const;

    QFile m_downloadFile;
    qint64 m_resumeFrom = 0;
    bool m_restartAttempted = false;
    Asset m_currentAsset;
    QString m_currentSavePath;

private:
    QString m_owner;
    QString m_repo;
    QString m_authToken;
    bool m_includePrereleases = false;

    QNetworkAccessManager m_nam;
    QPointer<QNetworkReply> m_activeReply;
    QPointer<QNetworkReply> m_downloadReply;
};
