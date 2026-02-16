#ifndef ARCHIVEEXTRACTOR_H
#define ARCHIVEEXTRACTOR_H

#include <QObject>
#include <QString>
#include <QStringList>

class ArchiveExtractor : public QObject {
    Q_OBJECT

public:
    explicit ArchiveExtractor(QObject *parent = nullptr);

    struct ExtractResult {
        bool success;
        QStringList pakFiles;
        QString tempDir;
        QString error;
    };

    ExtractResult extractPakFiles(const QString &zipPath);
    static bool isArchiveFile(const QString &filePath);

    static void cleanupTempDir(const QString &tempDir);

signals:
    void errorOccurred(const QString &error);

private:
    enum class ArchiveFormat {
        Zip, Rar, SevenZip, TarGz, TarBz2, TarXz, Unknown
    };

    ArchiveFormat detectFormat(const QString &filePath) const;
    ArchiveFormat detectFormatBySignature(const QString &filePath) const;
    ExtractResult extractWithLibarchive(const QString &archivePath);
    ExtractResult extractWithZip(const QString &archivePath);
    bool isPakFile(const QString &fileName) const;
    QString createTempDir() const;
};

#endif // ARCHIVEEXTRACTOR_H
