#ifndef NEXUSREGISTRATIONMODALCONTENT_H
#define NEXUSREGISTRATIONMODALCONTENT_H

#include "../BaseModalContent.h"
#include "../../utils/NexusFileInfo.h"
#include <QString>
#include <QList>

class NexusModsClient;
class NexusModsAuth;
class ModalManager;
class QLineEdit;
class QPushButton;
class QLabel;
class QStackedWidget;
class QListWidget;

class NexusRegistrationModalContent : public BaseModalContent {
    Q_OBJECT

public:
    explicit NexusRegistrationModalContent(NexusModsClient *client,
                                          NexusModsAuth *auth,
                                          ModalManager *modalManager,
                                          const QString &modId,
                                          const QString &modName,
                                          QWidget *parent = nullptr);

    QList<NexusFileInfo> getSelectedFiles() const { return m_selectedFiles; }
    QString getModId() const { return m_currentModId; }
    QString getAuthor() const { return m_author; }
    QString getDescription() const { return m_description; }

private slots:
    void onFetchClicked();
    void onAuthenticateClicked();
    void onAuthStarted(const QString &browserUrl);
    void onAuthComplete(const QString &apiKey);
    void onAuthFailed(const QString &error);
    void onModInfoReceived(const QString &author, const QString &description, const QString &version);
    void onModFilesReceived(const QList<NexusFileInfo> &files);
    void onError(const QString &error);

private:
    void setupUi();
    QWidget* createInputPage();
    QWidget* createAuthPage();
    void showInputPage();
    void showAuthPage();
    void updateFooterButtons();
    QString formatFileSize(qint64 bytes) const;

    NexusModsClient *m_client;
    NexusModsAuth *m_auth;
    ModalManager *m_modalManager;
    QString m_localModId;
    QString m_localModName;

    QStackedWidget *m_stack;
    QLineEdit *m_urlEdit;
    QPushButton *m_fetchButton;
    QPushButton *m_authenticateButton;
    QPushButton *m_cancelButton;
    QLabel *m_authStatusLabel;

    QList<NexusFileInfo> m_selectedFiles;

    QString m_currentModId;
    QString m_author;
    QString m_description;

    enum Page { InputPage, AuthPage };
};

#endif // NEXUSREGISTRATIONMODALCONTENT_H
