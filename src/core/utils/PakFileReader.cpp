#include "PakFileReader.h"
#include <QDataStream>
#include <QtEndian>

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

bool PakFileReader::extractFile(const QString &pakFilePath,
                                const QStringList &candidatePaths,
                                QByteArray *data,
                                QString *error) {
    if (!data) {
        if (error) {
            *error = "No output buffer provided";
        }
        return false;
    }

    QFile file(pakFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = "Failed to open file";
        }
        return false;
    }

    PakFooter footer;
    if (!readFooter(file, footer)) {
        if (error) {
            *error = "Failed to read footer or invalid .pak file";
        }
        return false;
    }

    if (footer.magic != PakFooter::MAGIC) {
        if (error) {
            *error = "Not a valid .pak file";
        }
        return false;
    }

    if (footer.version != 1 && footer.version != 2 && footer.version != 3 &&
        footer.version != 4 && footer.version != 7) {
        if (error) {
            *error = QString("Unsupported .pak version: %1").arg(footer.version);
        }
        return false;
    }

    if (!file.seek(footer.indexOffset)) {
        if (error) {
            *error = "Failed to seek to index";
        }
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    QString mountPoint = readString(file);
    if (mountPoint.isNull()) {
        if (error) {
            *error = "Failed to read mount point";
        }
        return false;
    }

    quint32 recordCount = 0;
    stream >> recordCount;
    if (stream.status() != QDataStream::Ok) {
        if (error) {
            *error = "Failed to read index record count";
        }
        return false;
    }

    QStringList candidatesLower;
    candidatesLower.reserve(candidatePaths.size());
    for (const QString &path : candidatePaths) {
        QString normalized = path;
        normalized.replace('\\', '/');
        candidatesLower.append(normalized.toLower());
    }

    FileEntry matchedEntry;
    bool found = false;

    for (quint32 i = 0; i < recordCount; ++i) {
        QString fileName = readString(file);
        if (fileName.isNull()) {
            if (error) {
                *error = "Failed to read index entry";
            }
            return false;
        }

        FileEntry entry;
        entry.path = fileName;

        if (!readEntry(file, footer.version, entry)) {
            if (error) {
                *error = "Failed to read entry metadata";
            }
            return false;
        }

        if (!found && matchesCandidate(fileName, candidatesLower)) {
            matchedEntry = entry;
            found = true;
        }
    }

    if (!found) {
        if (error) {
            *error = "Manifest not found in pak";
        }
        return false;
    }

    if (matchedEntry.encrypted) {
        if (error) {
            *error = "Manifest is encrypted in pak";
        }
        return false;
    }

    if (matchedEntry.uncompressedSize == 0 || matchedEntry.compressedSize == 0) {
        if (error) {
            *error = "Manifest entry has invalid size";
        }
        return false;
    }

    static constexpr quint64 kMaxManifestSize = 512 * 1024;
    if (matchedEntry.uncompressedSize > kMaxManifestSize) {
        if (error) {
            *error = "Manifest is too large";
        }
        return false;
    }

    // Each entry in the data section starts with an embedded record header.
    // The header mirrors the index entry fields (for the uncompressed state).
    // offset(8) + compressedSize(8) + uncompressedSize(8) + method(4) + sha1(20)
    quint64 headerSize = 8 + 8 + 8 + 4 + 20;
    if (footer.version <= 1) {
        headerSize += 8; // timestamp
    }
    if (footer.version >= 3) {
        headerSize += 1; // encrypted flag
        headerSize += 4; // compression block size
    }

    if (matchedEntry.compressionMethod != 0) {
        if (matchedEntry.compressionMethod != 1) {
            if (error) {
                *error = QString("Unsupported compression method: %1").arg(matchedEntry.compressionMethod);
            }
            return false;
        }

        if (matchedEntry.compressionBlocks.isEmpty() || matchedEntry.compressionBlockSize == 0) {
            if (error) {
                *error = "Compressed entry has no block info";
            }
            return false;
        }

        QByteArray decompressed;
        decompressed.reserve(static_cast<int>(matchedEntry.uncompressedSize));
        quint64 remaining = matchedEntry.uncompressedSize;

        for (const auto &[blockStart, blockEnd] : matchedEntry.compressionBlocks) {
            auto blockCompressedSize = static_cast<qint64>(blockEnd - blockStart);

            if (!file.seek(static_cast<qint64>(blockStart))) {
                if (error) {
                    *error = "Failed to seek to compression block";
                }
                return false;
            }

            QByteArray compressed = file.read(blockCompressedSize);
            if (compressed.size() != blockCompressedSize) {
                if (error) {
                    *error = "Failed to read compression block";
                }
                return false;
            }

            auto expectedBlockSize = static_cast<quint32>(
                qMin(remaining, static_cast<quint64>(matchedEntry.compressionBlockSize)));

            QByteArray prefixed;
            prefixed.reserve(4 + compressed.size());
            quint32 beSz = qToBigEndian(expectedBlockSize);
            prefixed.append(reinterpret_cast<const char *>(&beSz), 4);
            prefixed.append(compressed);

            QByteArray blockData = qUncompress(prefixed);
            if (blockData.isEmpty()) {
                if (error) {
                    *error = "Failed to decompress zlib block";
                }
                return false;
            }

            decompressed.append(blockData);
            remaining -= static_cast<quint64>(blockData.size());
        }

        if (static_cast<quint64>(decompressed.size()) != matchedEntry.uncompressedSize) {
            if (error) {
                *error = "Decompressed size mismatch";
            }
            return false;
        }

        *data = decompressed;
        return true;
    }

    if (!file.seek(static_cast<qint64>(matchedEntry.offset + headerSize))) {
        if (error) {
            *error = "Failed to seek to manifest data";
        }
        return false;
    }

    auto dataSize = static_cast<qint64>(matchedEntry.uncompressedSize);
    QByteArray buffer = file.read(dataSize);
    if (buffer.size() != dataSize) {
        if (error) {
            *error = "Failed to read manifest data";
        }
        return false;
    }

    *data = buffer;
    return true;
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

bool PakFileReader::readEntry(QFile &file, quint32 version, FileEntry &entry) {
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream >> entry.offset;
    stream >> entry.compressedSize;
    stream >> entry.uncompressedSize;
    stream >> entry.compressionMethod;

    if (version <= 1) {
        quint64 timestamp = 0;
        stream >> timestamp;
    }

    quint8 hash[20];
    for (int i = 0; i < 20; ++i) {
        stream >> hash[i];
    }

    if (version >= 3) {
        if (entry.compressionMethod != 0) {
            quint32 blockCount = 0;
            stream >> blockCount;
            entry.compressionBlocks.reserve(blockCount);
            for (quint32 i = 0; i < blockCount; ++i) {
                quint64 blockStart = 0;
                quint64 blockEnd = 0;
                stream >> blockStart;
                stream >> blockEnd;
                entry.compressionBlocks.append({blockStart, blockEnd});
            }
        }

        quint8 encrypted = 0;
        stream >> encrypted;
        entry.encrypted = (encrypted != 0);

        quint32 blockSize = 0;
        stream >> blockSize;
        entry.compressionBlockSize = blockSize;
    }

    return stream.status() == QDataStream::Ok;
}

bool PakFileReader::matchesCandidate(const QString &fileName, const QStringList &candidatesLower) {
    QString normalized = fileName;
    normalized.replace('\\', '/');
    QString lower = normalized.toLower();
    for (const QString &candidate : candidatesLower) {
        if (lower == candidate) {
            return true;
        }
        if (lower.endsWith('/' + candidate)) {
            return true;
        }
    }
    return false;
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
