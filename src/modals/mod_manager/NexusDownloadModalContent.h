#ifndef NEXUSDOWNLOADMODALCONTENT_H
#define NEXUSDOWNLOADMODALCONTENT_H

#include "common/modals/BaseModalContent.h"
#include "core/models/NexusFileInfo.h"
#include <QString>
#include <QList>

class QEvent;
class NexusModsClient;
class NexusModsAuth;
class ModalManager;
class QLineEdit;
class QPushButton;
class QProgressBar;
class QLabel;
class QStackedWidget;

class NexusDownloadModalContent : public BaseModalContent {
    Q_OBJECT

public:
    explicit NexusDownloadModalContent(NexusModsClient *client,
                                      NexusModsAuth *auth,
                                      ModalManager *modalManager,
                                      QWidget *parent = nullptr);

    QStringList getDownloadedFilePaths() const { return m_downloadedFilePaths; }
    QList<NexusFileInfo> getDownloadedFiles() const { return m_downloadedFiles; }
    QString getModId() const { return m_currentModId; }
    QString getAuthor() const { return m_author; }
    QString getDescription() const { return m_description; }

private slots:
    void onDownloadClicked();
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

protected:
    void changeEvent(QEvent *event) override;

private:
    void setupUi();
    void retranslateUi();
    QWidget* createInputPage();
    QWidget* createAuthPage();
    QWidget* createDownloadPage();
    void showInputPage();
    void showAuthPage();
    void showDownloadPage();
    void startDownloadProcess();
    void startNextDownload();
    void startManualDownloadSequence();
    QString generateTempPath(const QString &fileName) const;
    void updateFooterButtons();
    QString formatFileSize(qint64 bytes) const;

    NexusModsClient *m_client;
    NexusModsAuth *m_auth;
    ModalManager *m_modalManager;

    QStackedWidget *m_stack;
    QLineEdit *m_urlEdit;
    QPushButton *m_downloadButton;
    QPushButton *m_authenticateButton;
    QPushButton *m_cancelButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_authStatusLabel;

    QLabel *m_inputInstructionLabel;
    QLabel *m_authInstructionLabel;

    QStringList m_selectedFileIds;
    QList<NexusFileInfo> m_selectedFiles;
    QStringList m_downloadedFilePaths;
    QList<NexusFileInfo> m_downloadedFiles;
    int m_currentDownloadIndex;

    QString m_currentModId;
    QString m_currentFileId;
    QString m_pendingUrl;
    QString m_author;
    QString m_description;

    enum Page { InputPage, AuthPage, DownloadPage };
};

#endif // NEXUSDOWNLOADMODALCONTENT_H
