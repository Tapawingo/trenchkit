#ifndef PAKFILEREADER_H
#define PAKFILEREADER_H

#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QFile>
#include <QVector>
#include <QPair>

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

    struct FileEntry {
        QString path;
        quint64 offset = 0;
        quint64 compressedSize = 0;
        quint64 uncompressedSize = 0;
        quint32 compressionMethod = 0;
        bool encrypted = false;
        quint32 compressionBlockSize = 0;
        QVector<QPair<quint64, quint64>> compressionBlocks;
    };

    static ParseResult extractFilePaths(const QString &pakFilePath);
    static bool extractFile(const QString &pakFilePath,
                            const QStringList &candidatePaths,
                            QByteArray *data,
                            QString *error = nullptr);

private:
    static bool readFooter(QFile &file, PakFooter &footer);
    static bool readIndex(QFile &file, quint64 indexOffset, quint32 version, QStringList &filePaths, QString &mountPoint);
    static bool readEntry(QFile &file, quint32 version, FileEntry &entry);
    static bool matchesCandidate(const QString &fileName, const QStringList &candidatesLower);
    static QString readString(QIODevice &device);
    static quint64 calculateRecordMetadataSize(quint32 version);
};

#endif // PAKFILEREADER_H
