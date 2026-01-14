#include "PakFileReader.h"
#include <QDebug>
#include <QDataStream>

PakFileReader::ParseResult PakFileReader::extractFilePaths(const QString &pakFilePath) {
    ParseResult result;
    result.success = false;

    QFile file(pakFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.error = "Failed to open file";
        return result;
    }

    PakFooter footer;
    if (!readFooter(file, footer)) {
        result.error = "Failed to read footer or invalid .pak file";
        return result;
    }

    if (footer.magic != PakFooter::MAGIC) {
        result.error = "Not a valid .pak file";
        return result;
    }

    if (footer.version != 1 && footer.version != 2 && footer.version != 3 &&
        footer.version != 4 && footer.version != 7) {
        result.error = QString("Unsupported .pak version: %1").arg(footer.version);
        return result;
    }

    if (!readIndex(file, footer.indexOffset, footer.version, result.filePaths, result.mountPoint)) {
        result.error = "Failed to read index";
        return result;
    }

    result.success = true;
    return result;
}

bool PakFileReader::readFooter(QFile &file, PakFooter &footer) {
    if (file.size() < PakFooter::SIZE) {
        return false;
    }

    if (!file.seek(file.size() - PakFooter::SIZE)) {
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream >> footer.magic;
    stream >> footer.version;
    stream >> footer.indexOffset;
    stream >> footer.indexSize;

    for (int i = 0; i < 20; ++i) {
        stream >> footer.indexHash[i];
    }

    return stream.status() == QDataStream::Ok;
}

bool PakFileReader::readIndex(QFile &file, quint64 indexOffset, quint32 version,
                              QStringList &filePaths, QString &mountPoint) {
    if (!file.seek(indexOffset)) {
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    mountPoint = readString(file);
    if (mountPoint.isNull()) {
        return false;
    }

    quint32 recordCount;
    stream >> recordCount;

    if (stream.status() != QDataStream::Ok) {
        return false;
    }

    filePaths.reserve(recordCount);

    for (quint32 i = 0; i < recordCount; ++i) {
        QString fileName = readString(file);
        if (fileName.isNull()) {
            return false;
        }

        filePaths.append(fileName);

        quint64 metadataSize = calculateRecordMetadataSize(version);
        if (metadataSize > 0) {
            if (!file.seek(file.pos() + metadataSize)) {
                return false;
            }
        } else {
            quint64 dataOffset, compressedSize, uncompressedSize;
            quint32 compressionMethod;

            stream >> dataOffset;
            stream >> compressedSize;
            stream >> uncompressedSize;
            stream >> compressionMethod;

            if (version <= 1) {
                quint64 timestamp;
                stream >> timestamp;
            }

            quint8 hash[20];
            for (int j = 0; j < 20; ++j) {
                stream >> hash[j];
            }

            if (version >= 3) {
                if (compressionMethod != 0) {
                    quint32 blockCount;
                    stream >> blockCount;

                    for (quint32 b = 0; b < blockCount; ++b) {
                        quint64 blockStart, blockEnd;
                        stream >> blockStart;
                        stream >> blockEnd;
                    }
                }

                quint8 encrypted;
                stream >> encrypted;

                quint32 blockSize;
                stream >> blockSize;
            }

            if (stream.status() != QDataStream::Ok) {
                return false;
            }
        }
    }

    return true;
}

QString PakFileReader::readString(QIODevice &device) {
    QDataStream stream(&device);
    stream.setByteOrder(QDataStream::LittleEndian);

    quint32 length;
    stream >> length;

    if (stream.status() != QDataStream::Ok || length == 0 || length > 1024 * 1024) {
        return QString();
    }

    QByteArray data(length, '\0');
    if (stream.readRawData(data.data(), length) != static_cast<int>(length)) {
        return QString();
    }

    return QString::fromUtf8(data.data(), length - 1);
}

quint64 PakFileReader::calculateRecordMetadataSize(quint32 version) {
    switch (version) {
        case 1:
            return 8 + 8 + 8 + 4 + 8 + 20;
        case 2:
            return 8 + 8 + 8 + 4 + 20;
        default:
            return 0;
    }
}
