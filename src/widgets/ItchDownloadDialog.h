#pragma once

#include <QDialog>
#include <QString>
#include <QPointer>
#include "../utils/ItchUploadInfo.h"

class ItchClient;
class ItchAuth;
class QLineEdit;
class QPushButton;
class QProgressBar;
class QLabel;
class QStackedWidget;
class QVBoxLayout;

class ItchDownloadDialog : public QDialog {
    Q_OBJECT

public:
    explicit ItchDownloadDialog(ItchClient *client,
                               ItchAuth *auth,
                               QWidget *parent = nullptr);
    ~ItchDownloadDialog() override = default;

    QString getDownloadedFilePath() const { return m_downloadedPath; }
    QStringList getDownloadedFilePaths() const { return m_downloadedPaths; }
    QString getGameId() const { return m_currentGameId; }
    QString getAuthor() const { return m_author; }
    QString getGameTitle() const { return m_gameTitle; }

signals:
    void downloadComplete(QString filePath);

private slots:
    void onDownloadClicked();
    void onCancelClicked();
    void onApiKeySubmit();
    void onGameIdReceived(const QString &gameId, const QString &title, const QString &author);
    void onUploadsReceived(const QList<ItchUploadInfo> &uploads);
    void onDownloadLinkReceived(const QString &url);
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished(const QString &savePath);
    void onError(const QString &error);

private:
    void setupUi();
    QWidget* createInputPage();
    QWidget* createAuthPage();
    QWidget* createDownloadPage();
    void showInputPage();
    void showAuthPage();
    void showDownloadPage();
    void startDownloadProcess();
    void startNextDownload();
    QString generateTempPath(const QString &fileName) const;

    ItchClient *m_client;
    ItchAuth *m_auth;

    QStackedWidget *m_stack;
    QLineEdit *m_urlEdit;
    QLineEdit *m_apiKeyEdit;
    QPushButton *m_downloadButton;
    QPushButton *m_cancelButton;
    QPushButton *m_submitApiKeyButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_authInstructionLabel;

    QString m_downloadedPath;
    QString m_currentGameId;
    QString m_currentUploadId;
    QString m_pendingCreator;
    QString m_pendingGameName;
    QString m_author;
    QString m_gameTitle;

    QList<ItchUploadInfo> m_pendingUploads;
    int m_currentDownloadIndex;
    QStringList m_downloadedPaths;

    enum Page { InputPage, AuthPage, DownloadPage };
};
