#pragma once

#include <QObject>
#include <QWebSocket>
#include <QString>

class NexusModsAuth final : public QObject {
    Q_OBJECT

public:
    explicit NexusModsAuth(QObject *parent = nullptr);
    ~NexusModsAuth() override = default;

public slots:
    void startAuthentication();
    void cancelAuthentication();

signals:
    void authenticationStarted(QString browserUrl);
    void authenticationComplete(QString apiKey);
    void authenticationFailed(QString error);
    void authenticationCancelled();

private slots:
    void onConnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);

private:
    void cleanup();
    QString generateConnectionToken() const;

    QWebSocket m_socket;
    QString m_uuid;
    QString m_token;
    bool m_isAuthenticating = false;
};
