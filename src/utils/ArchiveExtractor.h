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

    static void cleanupTempDir(const QString &tempDir);

signals:
    void errorOccurred(const QString &error);

private:
    bool isPakFile(const QString &fileName) const;
    QString createTempDir() const;
};

#endif // ARCHIVEEXTRACTOR_H
