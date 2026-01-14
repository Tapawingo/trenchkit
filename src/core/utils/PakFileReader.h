#ifndef PAKFILEREADER_H
#define PAKFILEREADER_H

#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QFile>

class PakFileReader {
public:
    struct PakFooter {
        quint32 magic;
        quint32 version;
        quint64 indexOffset;
        quint64 indexSize;
        quint8 indexHash[20];

        static constexpr quint32 MAGIC = 0x5A6F12E1;
        static constexpr int SIZE = 44;
    };

    struct ParseResult {
        bool success;
        QString error;
        QStringList filePaths;
        QString mountPoint;
    };

    static ParseResult extractFilePaths(const QString &pakFilePath);

private:
    static bool readFooter(QFile &file, PakFooter &footer);
    static bool readIndex(QFile &file, quint64 indexOffset, quint32 version, QStringList &filePaths, QString &mountPoint);
    static QString readString(QIODevice &device);
    static quint64 calculateRecordMetadataSize(quint32 version);
};

#endif // PAKFILEREADER_H
