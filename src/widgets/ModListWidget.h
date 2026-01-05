#ifndef MODLISTWIDGET_H
#define MODLISTWIDGET_H

#include "../utils/ModManager.h"
#include "DraggableModList.h"
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

class NexusModsClient;
class NexusModsAuth;
class ItchClient;
class ItchAuth;
class ModUpdateService;
struct ModUpdateInfo;

class ModListWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModListWidget(QWidget *parent = nullptr);
    ~ModListWidget() override = default;

    void setModManager(ModManager *modManager);
    void setNexusServices(NexusModsClient *client, NexusModsAuth *auth);
    void setItchServices(ItchClient *client, ItchAuth *auth);
    void setUpdateService(ModUpdateService *service);
    void refreshModList();
    void setLoadingState(bool loading, const QString &message = "Loading mods");

signals:
    void modSelectionChanged(int selectedRow, int totalMods);
    void modAdded(const QString &modName);
    void modRemoved(const QString &modName);
    void modReordered();

public slots:
    void onAddModClicked();
    void onRemoveModClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onCheckUpdatesClicked();

private slots:
    void onModsChanged();
    void onModEnabledChanged(const QString &modId, bool enabled);
    void onItemsReordered();
    void updateLoadingAnimation();
    void onRenameRequested(const QString &modId);
    void onEditMetaRequested(const QString &modId);
    void onRemoveRequested(const QString &modId);
    void onUpdateRequested(const QString &modId);
    void onUpdateFound(const QString &modId, const ModUpdateInfo &updateInfo);
    void onUpdateCheckComplete(int updatesFound);

private:
    void setupUi();
    QString getSelectedModId() const;
    int getSelectedRow() const;

    ModManager *m_modManager = nullptr;
    NexusModsClient *m_nexusClient = nullptr;
    NexusModsAuth *m_nexusAuth = nullptr;
    ItchClient *m_itchClient = nullptr;
    ItchAuth *m_itchAuth = nullptr;
    ModUpdateService *m_updateService = nullptr;
    DraggableModList *m_modList;

    QLabel *m_loadingLabel;
    QLabel *m_modCountLabel;
    QPushButton *m_checkUpdatesButton;
    QTimer *m_loadingTimer;
    int m_loadingDots = 0;

    bool m_updating = false;
};

#endif // MODLISTWIDGET_H
