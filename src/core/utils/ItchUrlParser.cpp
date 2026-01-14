#include "ItchUrlParser.h"
#include <QUrl>
#include <QRegularExpression>

ItchUrlParser::ParseResult ItchUrlParser::parseUrl(const QString &url) {
    ParseResult result;

    QUrl qurl(url);
    if (!qurl.isValid()) {
        result.error = QStringLiteral("Invalid URL format");
        return result;
    }

    QString host = qurl.host();

    // itch.io URLs can be: https://creator.itch.io/game or https://itch.io/...
    // We want the pattern: creator.itch.io
    if (!host.endsWith(QStringLiteral(".itch.io")) && host != QStringLiteral("itch.io")) {
        result.error = QStringLiteral("URL must be from itch.io");
        return result;
    }

    // Extract creator from subdomain
    QRegularExpression creatorPattern(QStringLiteral("^([^.]+)\\.itch\\.io$"));
    QRegularExpressionMatch creatorMatch = creatorPattern.match(host);

    if (!creatorMatch.hasMatch()) {
        result.error = QStringLiteral("Invalid itch.io URL format. Expected: https://creator.itch.io/game");
        return result;
    }

    result.creator = creatorMatch.captured(1);

    // Extract game name from path (e.g., /game-name)
    QString path = qurl.path();
    if (path.startsWith('/')) {
        path = path.mid(1);  // Remove leading slash
    }

    // Remove trailing slash if present
    if (path.endsWith('/')) {
        path.chop(1);
    }

    // Game name should not be empty or contain additional slashes
    if (path.isEmpty()) {
        result.error = QStringLiteral("Game name not found in URL. Expected: https://creator.itch.io/game");
        return result;
    }

    if (path.contains('/')) {
        result.error = QStringLiteral("Invalid game URL. Expected: https://creator.itch.io/game");
        return result;
    }

    result.gameName = path;
    result.isValid = true;
    return result;
}
