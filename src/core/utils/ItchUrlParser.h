#pragma once

#include <QString>

class ItchUrlParser {
public:
    struct ParseResult {
        bool isValid = false;
        QString creator;      // Creator/developer name
        QString gameName;     // Game slug name
        QString error;
    };

    static ParseResult parseUrl(const QString &url);

private:
    ItchUrlParser() = delete;
};
