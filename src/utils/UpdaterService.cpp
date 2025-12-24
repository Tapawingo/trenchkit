#include "UpdaterService.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QSslSocket>

static QString trimmed(const QString& s) {
    auto t = s.trimmed();
    return t;
}

UpdaterService::UpdaterService(QString owner, QString repo, QObject* parent)
    : QObject(parent),
      m_owner(std::move(owner)),
      m_repo(std::move(repo)) {}

void UpdaterService::setAuthToken(const QString& token) {
    m_authToken = token;
}

void UpdaterService::setRepository(const QString& owner, const QString& repo) {
    if (owner.isEmpty() || repo.isEmpty()) {
        return;
    }
    m_owner = owner;
    m_repo = repo;
}

void UpdaterService::setIncludePrereleases(bool include) {
    m_includePrereleases = include;
}

QVersionNumber UpdaterService::currentVersion() const {
#ifdef TRENCHKIT_VERSION
    return QVersionNumber::fromString(QStringLiteral(TRENCHKIT_VERSION));
#else
    return QVersionNumber::fromString(QStringLiteral("0.0.0"));
#endif
}

QNetworkRequest UpdaterService::makeRequest(const QUrl& url) const {
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("TrenchKit-Updater"));
    req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("Accept", "application/vnd.github+json");
    if (!m_authToken.isEmpty()) {
        req.setRawHeader("Authorization", QByteArray("Bearer ") + m_authToken.toUtf8());
    }
    return req;
}

QVersionNumber UpdaterService::parseVersionFromTag(const QString& tag) {
    QString t = trimmed(tag);
    if (t.startsWith('v') || t.startsWith('V')) t = t.mid(1);
    QString cleaned;
    cleaned.reserve(t.size());
    for (QChar c : t) {
        if (c.isDigit() || c == '.') cleaned.append(c);
        else break;
    }
    return QVersionNumber::fromString(cleaned);
}

void UpdaterService::checkForUpdates() {
    emit checkingStarted();

    if (!QSslSocket::supportsSsl()) {
        const QString message = QStringLiteral("TLS initialization failed. No TLS backend is available.");
        qWarning() << "Updater:" << message
                   << "Build SSL:" << QSslSocket::sslLibraryBuildVersionString()
                   << "Runtime SSL:" << QSslSocket::sslLibraryVersionString();
        emit errorOccurred(message);
        return;
    }

    qInfo() << "Updater: checking for updates.";
    const QUrl url = m_includePrereleases
        ? QUrl(QStringLiteral("https://api.github.com/repos/%1/%2/releases")
                   .arg(m_owner, m_repo))
        : QUrl(QStringLiteral("https://api.github.com/repos/%1/%2/releases/latest")
                   .arg(m_owner, m_repo));

    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply->deleteLater();
    }

    m_activeReply = m_nam.get(makeRequest(url));
    connect(m_activeReply, &QNetworkReply::finished, this, [this]() {
        if (!m_activeReply) return;

        QNetworkReply* reply = m_activeReply;
        const auto err = reply->error();
        const QByteArray body = reply->readAll();
        const int httpStatus =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        m_activeReply->deleteLater();
        m_activeReply = nullptr;

        if (err != QNetworkReply::NoError || (httpStatus != 200 && httpStatus != 0)) {
            if (!m_includePrereleases && httpStatus == 404) {
                const QUrl fallbackUrl(QStringLiteral("https://api.github.com/repos/%1/%2/releases")
                                           .arg(m_owner, m_repo));
                qInfo() << "Updater: latest release not found, falling back to releases list.";
                m_activeReply = m_nam.get(makeRequest(fallbackUrl));
                connect(m_activeReply, &QNetworkReply::finished, this, [this]() {
                    if (!m_activeReply) return;

                    QNetworkReply* fallbackReply = m_activeReply;
                    const auto fallbackErr = fallbackReply->error();
                    const QByteArray fallbackBody = fallbackReply->readAll();
                    const int fallbackStatus =
                        fallbackReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    m_activeReply->deleteLater();
                    m_activeReply = nullptr;

                    if (fallbackErr != QNetworkReply::NoError || (fallbackStatus != 200 && fallbackStatus != 0)) {
                        emit errorOccurred(formatNetworkError(
                            QStringLiteral("Update check failed"), fallbackReply, fallbackBody));
                        return;
                    }

                    handleReleaseJson(fallbackBody);
                });
                return;
            }

            emit errorOccurred(formatNetworkError(
                QStringLiteral("Update check failed"), reply, body));
            return;
        }

        handleReleaseJson(body);
    });
}

void UpdaterService::handleReleaseJson(const QByteArray& json) {
    const auto doc = QJsonDocument::fromJson(json);
    if (doc.isNull()) {
        emit errorOccurred(QStringLiteral("Update check failed: invalid JSON response."));
        return;
    }

    QJsonObject relObj;
    if (doc.isObject()) {
        relObj = doc.object();
    } else if (doc.isArray()) {
        const auto arr = doc.array();
        for (const auto& v : arr) {
            if (!v.isObject()) continue;
            const auto o = v.toObject();
            const bool draft = o.value("draft").toBool();
            const bool prerelease = o.value("prerelease").toBool();
            if (draft) continue;
            if (!m_includePrereleases && prerelease) continue;
            relObj = o;
            break;
        }
        if (relObj.isEmpty()) {
            emit errorOccurred(QStringLiteral("No suitable releases found."));
            return;
        }
    } else {
        emit errorOccurred(QStringLiteral("Update check failed: unexpected JSON format."));
        return;
    }

    ReleaseInfo info;
    info.tagName = relObj.value("tag_name").toString();
    info.name = relObj.value("name").toString();
    info.body = relObj.value("body").toString();
    info.prerelease = relObj.value("prerelease").toBool();
    info.draft = relObj.value("draft").toBool();
    info.htmlUrl = QUrl(relObj.value("html_url").toString());
    info.publishedAt = QDateTime::fromString(relObj.value("published_at").toString(), Qt::ISODate);

    info.version = parseVersionFromTag(info.tagName);

    const auto assets = relObj.value("assets").toArray();
    for (const auto& a : assets) {
        if (!a.isObject()) continue;
        const auto ao = a.toObject();

        Asset asset;
        asset.name = ao.value("name").toString();
        asset.downloadUrl = QUrl(ao.value("browser_download_url").toString());
        asset.sizeBytes = static_cast<qint64>(ao.value("size").toDouble(-1));
        asset.contentType = ao.value("content_type").toString();
        info.assets.push_back(asset);
    }

    const auto cur = currentVersion();
    if (!info.version.isNull() && QVersionNumber::compare(info.version, cur) > 0) {
        emit updateAvailable(info);
    } else {
        emit upToDate(info);
    }
}

void UpdaterService::downloadAsset(const Asset& asset, const QString& savePath) {
    if (!asset.downloadUrl.isValid()) {
        emit errorOccurred(QStringLiteral("Invalid download URL."));
        return;
    }

    m_currentAsset = asset;
    m_currentSavePath = savePath;
    m_restartAttempted = false;

    startDownload(true, false);
}

void UpdaterService::startDownload(bool allowResume, bool forceRestart) {
    if (!m_currentAsset.downloadUrl.isValid()) {
        emit errorOccurred(QStringLiteral("Invalid download URL."));
        return;
    }

    qInfo() << "Updater: starting download for" << m_currentAsset.name;
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }

    m_downloadFile.close();
    m_resumeFrom = 0;

    QFileInfo fi(m_currentSavePath);
    if (!fi.dir().exists()) {
        fi.dir().mkpath(".");
    }

    m_downloadFile.setFileName(m_currentSavePath);
    if (!forceRestart && allowResume && m_downloadFile.exists()) {
        m_resumeFrom = m_downloadFile.size();
        if (!m_downloadFile.open(QIODevice::Append)) {
            emit errorOccurred(QStringLiteral("Failed to open file for resume."));
            return;
        }
    } else {
        if (!m_downloadFile.open(QIODevice::WriteOnly)) {
            emit errorOccurred(QStringLiteral("Failed to create file."));
            return;
        }
    }

    QNetworkRequest req = makeRequest(m_currentAsset.downloadUrl);

    if (m_resumeFrom > 0) {
        req.setRawHeader("Range",
            QByteArray("bytes=") + QByteArray::number(m_resumeFrom) + "-");
    }

    m_downloadReply = m_nam.get(req);

    connect(m_downloadReply, &QNetworkReply::metaDataChanged, this, [this]() {
        if (!m_downloadReply || m_resumeFrom <= 0 || m_restartAttempted) return;
        const int httpStatus =
            m_downloadReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpStatus == 200) {
            qInfo() << "Updater: server ignored Range, restarting download.";
            restartDownloadFromScratch();
        }
    });

    connect(m_downloadReply, &QNetworkReply::downloadProgress,
            this, [this](qint64 received, qint64 total) {
        if (total > 0) {
            emit downloadProgress(m_resumeFrom + received,
                                  m_resumeFrom + total);
        }
    });

    connect(m_downloadReply, &QNetworkReply::readyRead, this, [this]() {
        if (!m_downloadReply || !m_downloadFile.isOpen()) return;
        m_downloadFile.write(m_downloadReply->readAll());
    });

    connect(m_downloadReply, &QNetworkReply::finished, this, [this]() {
        if (!m_downloadReply) return;

        QNetworkReply* reply = m_downloadReply;
        const auto err = reply->error();
        const int httpStatus =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();

        reply->deleteLater();
        m_downloadReply = nullptr;

        m_downloadFile.flush();
        m_downloadFile.close();

        if (err != QNetworkReply::NoError) {
            emit errorOccurred(formatNetworkError(
                QStringLiteral("Download failed"), reply, body));
            return;
        }

        if (httpStatus != 200 && httpStatus != 206) {
            emit errorOccurred(QStringLiteral("Unexpected HTTP status: %1").arg(httpStatus));
            return;
        }

        if (m_resumeFrom > 0 && httpStatus == 200 && !m_restartAttempted) {
            emit errorOccurred(QStringLiteral("Server returned full response during resume."));
            return;
        }

        emit downloadFinished(m_currentSavePath);
    });
}

void UpdaterService::restartDownloadFromScratch() {
    if (m_restartAttempted) return;
    m_restartAttempted = true;
    startDownload(false, true);
}

void UpdaterService::cancelDownload() {
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
    if (m_downloadFile.isOpen()) {
        m_downloadFile.flush();
        m_downloadFile.close();
    }
}

QString UpdaterService::formatNetworkError(const QString& context,
                                           QNetworkReply* reply,
                                           const QByteArray& body) const {
    QStringList parts;
    parts << context;
    if (reply) {
        const int httpStatus =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpStatus > 0) {
            parts << QStringLiteral("HTTP %1").arg(httpStatus);
        }
        const QString err = reply->errorString().trimmed();
        if (!err.isEmpty()) {
            parts << err;
        }
    }
    const QString snippet = QString::fromUtf8(body.left(200)).trimmed();
    if (!snippet.isEmpty()) {
        QString cleaned = snippet;
        cleaned.replace('\n', ' ');
        cleaned.replace('\r', ' ');
        parts << QStringLiteral("Response: %1").arg(cleaned);
    }
    return parts.join(QStringLiteral(" - "));
}
