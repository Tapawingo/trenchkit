#pragma once

#include <QDialog>
#include <QString>
#include <QPointer>
#include "../utils/NexusFileInfo.h"

class NexusModsClient;
class NexusModsAuth;
class QLineEdit;
class QPushButton;
class QProgressBar;
class QLabel;
class QStackedWidget;
class QVBoxLayout;

class NexusDownloadDialog : public QDialog {
    Q_OBJECT

public:
    explicit NexusDownloadDialog(NexusModsClient *client,
                                 NexusModsAuth *auth,
                                 QWidget *parent = nullptr);
    ~NexusDownloadDialog() override = default;

    QString getDownloadedFilePath() const { return m_downloadedPath; }
    QString getModId() const { return m_currentModId; }
    QString getFileId() const { return m_currentFileId; }
    QString getAuthor() const { return m_author; }
    QString getDescription() const { return m_description; }
    QString getVersion() const { return m_version; }

signals:
    void downloadComplete(QString filePath);

private slots:
    void onDownloadClicked();
    void onCancelClicked();
    void onAuthenticateClicked();
    void onAuthStarted(const QString &browserUrl);
    void onAuthComplete(const QString &apiKey);
    void onAuthFailed(const QString &error);
    void onModInfoReceived(const QString &author, const QString &description, const QString &version);
    void onModFilesReceived(const QList<NexusFileInfo> &files);
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
    QString generateTempPath(const QString &fileName) const;

    NexusModsClient *m_client;
    NexusModsAuth *m_auth;

    QStackedWidget *m_stack;
    QLineEdit *m_urlEdit;
    QPushButton *m_downloadButton;
    QPushButton *m_cancelButton;
    QPushButton *m_authenticateButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_authStatusLabel;

    QString m_downloadedPath;
    QString m_currentModId;
    QString m_currentFileId;
    QString m_pendingUrl;
    QString m_author;
    QString m_description;
    QString m_version;

    enum Page { InputPage, AuthPage, DownloadPage };
};
