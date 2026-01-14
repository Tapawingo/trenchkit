#include "NexusModsAuth.h"
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QDebug>

NexusModsAuth::NexusModsAuth(QObject *parent)
    : QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &NexusModsAuth::onConnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &NexusModsAuth::onTextMessageReceived);
    connect(&m_socket, &QWebSocket::disconnected, this, &NexusModsAuth::onDisconnected);
    connect(&m_socket, &QWebSocket::errorOccurred, this, &NexusModsAuth::onError);
}

void NexusModsAuth::startAuthentication() {
    if (m_isAuthenticating) {
        qDebug() << "[NexusAuth] Already authenticating, ignoring request";
        return;
    }

    qDebug() << "[NexusAuth] Starting authentication";
    m_isAuthenticating = true;
    m_uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_token = generateConnectionToken();

    qDebug() << "[NexusAuth] UUID:" << m_uuid;
    qDebug() << "[NexusAuth] Opening WebSocket to wss://sso.nexusmods.com";

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
    qDebug() << "[NexusAuth] WebSocket connected";
    QJsonObject connectionMessage;
    connectionMessage[QStringLiteral("id")] = m_uuid;
    connectionMessage[QStringLiteral("token")] = m_token;
    connectionMessage[QStringLiteral("protocol")] = 2;

    QJsonDocument doc(connectionMessage);
    QString jsonMsg = doc.toJson(QJsonDocument::Compact);
    qDebug() << "[NexusAuth] Sending connection message:" << jsonMsg;
    m_socket.sendTextMessage(jsonMsg);

    QString browserUrl = QStringLiteral("https://www.nexusmods.com/sso?id=%1&application=tapawingo-trenchkit")
                            .arg(m_uuid);
    qDebug() << "[NexusAuth] Browser URL:" << browserUrl;
    emit authenticationStarted(browserUrl);
}

void NexusModsAuth::onTextMessageReceived(const QString &message) {
    qDebug() << "[NexusAuth] Received message:" << message;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "[NexusAuth] Invalid JSON received";
        emit authenticationFailed(QStringLiteral("Invalid response from authentication server"));
        cleanup();
        return;
    }

    QJsonObject obj = doc.object();
    if (obj.contains(QStringLiteral("success")) && obj[QStringLiteral("success")].toBool()) {
        qDebug() << "[NexusAuth] Success response received";
        QJsonObject data = obj[QStringLiteral("data")].toObject();

        // Check if this is the connection acknowledgment or the actual authentication result
        if (data.contains(QStringLiteral("connection_token"))) {
            qDebug() << "[NexusAuth] Connection acknowledged, waiting for user authorization...";
            // This is just the connection acknowledgment, keep waiting for the actual API key
            return;
        }

        QString apiKey = data[QStringLiteral("api_key")].toString();
        if (apiKey.isEmpty()) {
            qDebug() << "[NexusAuth] API key is empty in authentication response";
            emit authenticationFailed(QStringLiteral("No API key received from server"));
            cleanup();
            return;
        }

        qDebug() << "[NexusAuth] Authentication complete with API key";
        emit authenticationComplete(apiKey);
        cleanup();
    } else if (obj.contains(QStringLiteral("error"))) {
        QString error = obj[QStringLiteral("error")].toString();
        qDebug() << "[NexusAuth] Error response:" << error;
        emit authenticationFailed(error.isEmpty() ? QStringLiteral("Authentication failed") : error);
        cleanup();
    } else {
        // Received a message but it wasn't success or error - might be a connection acknowledgment
        qDebug() << "[NexusAuth] Non-success/error message, waiting for actual response";
    }
}

void NexusModsAuth::onDisconnected() {
    qDebug() << "[NexusAuth] WebSocket disconnected, authenticating:" << m_isAuthenticating;
    if (!m_isAuthenticating) {
        return;
    }

    qDebug() << "[NexusAuth] Unexpected disconnect during authentication";
    emit authenticationFailed(QStringLiteral("Connection to authentication server was closed unexpectedly"));
    cleanup();
}

void NexusModsAuth::onError(QAbstractSocket::SocketError error) {
    qDebug() << "[NexusAuth] WebSocket error:" << error << m_socket.errorString();
    if (!m_isAuthenticating) {
        return;
    }

    emit authenticationFailed(m_socket.errorString());
    cleanup();
}

void NexusModsAuth::cleanup() {
    qDebug() << "[NexusAuth] Cleanup called, socket state:" << m_socket.state();
    m_isAuthenticating = false;
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        qDebug() << "[NexusAuth] Closing WebSocket connection";
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
