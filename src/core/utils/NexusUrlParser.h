#pragma once

#include <QString>

class NexusUrlParser {
public:
    struct ParseResult {
        bool isValid = false;
        QString gameDomain;
        QString modId;
        QString fileId;
        QString error;

        bool hasFileId() const { return !fileId.isEmpty(); }
    };

    static ParseResult parseUrl(const QString &url);

private:
    NexusUrlParser() = delete;
};
