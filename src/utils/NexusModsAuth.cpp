#include "NexusModsAuth.h"
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

NexusModsAuth::NexusModsAuth(QObject *parent)
    : QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &NexusModsAuth::onConnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &NexusModsAuth::onTextMessageReceived);
    connect(&m_socket, &QWebSocket::errorOccurred, this, &NexusModsAuth::onError);
}

void NexusModsAuth::startAuthentication() {
    if (m_isAuthenticating) {
        return;
    }

    m_isAuthenticating = true;
    m_uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_token = generateConnectionToken();

    QUrl wsUrl(QStringLiteral("wss://sso.nexusmods.com"));
    m_socket.open(wsUrl);
}

void NexusModsAuth::cancelAuthentication() {
    if (!m_isAuthenticating) {
        return;
    }

    cleanup();
    emit authenticationCancelled();
}

void NexusModsAuth::onConnected() {
    QJsonObject connectionMessage;
    connectionMessage[QStringLiteral("id")] = m_uuid;
    connectionMessage[QStringLiteral("token")] = m_token;
    connectionMessage[QStringLiteral("protocol")] = 2;

    QJsonDocument doc(connectionMessage);
    m_socket.sendTextMessage(doc.toJson(QJsonDocument::Compact));

    QString browserUrl = QStringLiteral("https://www.nexusmods.com/sso?id=%1&application=trenchkit")
                            .arg(m_uuid);
    emit authenticationStarted(browserUrl);
}

void NexusModsAuth::onTextMessageReceived(const QString &message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        emit authenticationFailed(QStringLiteral("Invalid response from authentication server"));
        cleanup();
        return;
    }

    QJsonObject obj = doc.object();
    if (obj.contains(QStringLiteral("success")) && obj[QStringLiteral("success")].toBool()) {
        QString apiKey = obj[QStringLiteral("data")].toObject()[QStringLiteral("api_key")].toString();
        if (apiKey.isEmpty()) {
            emit authenticationFailed(QStringLiteral("No API key received"));
            cleanup();
            return;
        }

        emit authenticationComplete(apiKey);
        cleanup();
    } else if (obj.contains(QStringLiteral("error"))) {
        QString error = obj[QStringLiteral("error")].toString();
        emit authenticationFailed(error.isEmpty() ? QStringLiteral("Authentication failed") : error);
        cleanup();
    }
}

void NexusModsAuth::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    if (!m_isAuthenticating) {
        return;
    }

    emit authenticationFailed(m_socket.errorString());
    cleanup();
}

void NexusModsAuth::cleanup() {
    m_isAuthenticating = false;
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        m_socket.close();
    }
    m_uuid.clear();
    m_token.clear();
}

QString NexusModsAuth::generateConnectionToken() const {
    QString token;
    token.reserve(32);

    for (int i = 0; i < 32; ++i) {
        quint32 value = QRandomGenerator::global()->bounded(16);
        token.append(QString::number(value, 16));
    }

    return token;
}
