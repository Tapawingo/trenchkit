#ifndef ADDMODMODALCONTENT_H
#define ADDMODMODALCONTENT_H

#include "../BaseModalContent.h"
#include "../../utils/ItchUploadInfo.h"
#include <QString>
#include <QDateTime>

class ModManager;
class NexusModsClient;
class NexusModsAuth;
class ItchClient;
class ItchAuth;
class ModalManager;
class QPushButton;

class AddModModalContent : public BaseModalContent {
    Q_OBJECT

public:
    explicit AddModModalContent(ModManager *modManager,
                               NexusModsClient *nexusClient,
                               NexusModsAuth *nexusAuth,
                               ItchClient *itchClient,
                               ItchAuth *itchAuth,
                               ModalManager *modalManager,
                               QWidget *parent = nullptr);

signals:
    void modAdded(const QString &modName);

private slots:
    void onFromFileClicked();
    void onFromNexusClicked();
    void onFromItchClicked();

private:
    void setupUi();
    void handleZipFile(const QString &zipPath, const QString &nexusModId = QString(), const QString &nexusFileId = QString(),
                       const QString &author = QString(), const QString &description = QString(), const QString &version = QString(),
                       const QString &itchGameId = QString(), const QDateTime &uploadDate = QDateTime());
    void handlePakFile(const QString &pakPath, const QString &nexusModId = QString(), const QString &nexusFileId = QString(),
                       const QString &author = QString(), const QString &description = QString(), const QString &version = QString(),
                       const QString &itchGameId = QString(), const QString &customModName = QString(), const QDateTime &uploadDate = QDateTime());

    ModManager *m_modManager;
    NexusModsClient *m_nexusClient;
    NexusModsAuth *m_nexusAuth;
    ItchClient *m_itchClient;
    ItchAuth *m_itchAuth;
    ModalManager *m_modalManager;
    QPushButton *m_fromFileButton;
    QPushButton *m_fromNexusButton;
    QPushButton *m_fromItchButton;
};

#endif // ADDMODMODALCONTENT_H
