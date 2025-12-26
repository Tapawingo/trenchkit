#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QFile>
#include "NexusFileInfo.h"

class NexusModsClient final : public QObject {
    Q_OBJECT

public:
    explicit NexusModsClient(QObject *parent = nullptr);
    ~NexusModsClient() override = default;

    bool hasApiKey() const;
    void setApiKey(const QString &key);
    void clearApiKey();
    QString getApiKey() const { return m_apiKey; }

public slots:
    void getModInfo(const QString &modId);
    void getModFiles(const QString &modId);
    void getDownloadLink(const QString &modId, const QString &fileId);
    void downloadFile(const QUrl &url, const QString &savePath);
    void cancelDownload();

signals:
    void modInfoReceived(QString author, QString description, QString version);
    void modFilesReceived(QList<NexusFileInfo> files);
    void downloadLinkReceived(QString downloadUrl);
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(QString savePath);
    void errorOccurred(QString message);
    void rateLimitExceeded(int secondsUntilReset);

private:
    QNetworkRequest makeRequest(const QUrl &url) const;
    QString formatNetworkError(const QString &context,
                              QNetworkReply *reply,
                              const QByteArray &body) const;
    void saveApiKey();
    void loadApiKey();
    void handleRateLimitHeaders(QNetworkReply *reply);

    QNetworkAccessManager m_nam;
    QPointer<QNetworkReply> m_modInfoReply;
    QPointer<QNetworkReply> m_filesReply;
    QPointer<QNetworkReply> m_linkReply;
    QPointer<QNetworkReply> m_downloadReply;

    QString m_apiKey;
    QFile m_downloadFile;

    static constexpr const char* GAME_DOMAIN = "foxhole";
    static constexpr const char* API_BASE = "https://api.nexusmods.com/v1";
};
