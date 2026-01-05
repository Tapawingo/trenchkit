#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QFile>
#include "ItchUploadInfo.h"

class ItchClient final : public QObject {
    Q_OBJECT

public:
    explicit ItchClient(QObject *parent = nullptr);
    ~ItchClient() override = default;

    bool hasApiKey() const;
    void setApiKey(const QString &key);
    void clearApiKey();
    QString getApiKey() const { return m_apiKey; }

public slots:
    void getGameId(const QString &creator, const QString &gameName);
    void getGameUploads(const QString &gameId);
    void getDownloadLink(const QString &uploadId);
    void downloadFile(const QUrl &url, const QString &savePath);
    void cancelDownload();

signals:
    void gameIdReceived(QString gameId, QString gameTitle, QString author);
    void uploadsReceived(QList<ItchUploadInfo> uploads);
    void downloadLinkReceived(QString downloadUrl);
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(QString savePath);
    void errorOccurred(QString message);

private:
    QNetworkRequest makeRequest(const QUrl &url, bool useAuth = true) const;
    QString formatNetworkError(const QString &context,
                              QNetworkReply *reply,
                              const QByteArray &body) const;
    void saveApiKey();
    void loadApiKey();

    QNetworkAccessManager m_nam;
    QPointer<QNetworkReply> m_gameIdReply;
    QPointer<QNetworkReply> m_uploadsReply;
    QPointer<QNetworkReply> m_linkReply;
    QPointer<QNetworkReply> m_downloadReply;

    QString m_apiKey;
    QFile m_downloadFile;

    static constexpr const char* API_BASE = "https://itch.io/api/1";
};
