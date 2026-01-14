#ifndef MODLISTWIDGET_H
#define MODLISTWIDGET_H

#include "core/managers/ModManager.h"
#include "DraggableModList.h"
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QRegularExpression>

class NexusModsClient;
class NexusModsAuth;
class ItchClient;
class ItchAuth;
class ModUpdateService;
class ItchModUpdateService;
class ModalManager;
class ModConflictDetector;
class QLineEdit;
class QShortcut;
struct ModUpdateInfo;
struct ItchUpdateInfo;
struct ConflictInfo;

class ModListWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModListWidget(QWidget *parent = nullptr);
    ~ModListWidget() override = default;

    void setModManager(ModManager *modManager);
    void setNexusServices(NexusModsClient *client, NexusModsAuth *auth);
    void setItchServices(ItchClient *client, ItchAuth *auth);
    void setUpdateService(ModUpdateService *service);
    void setItchUpdateService(ItchModUpdateService *service);
    void setModalManager(ModalManager *modalManager) { m_modalManager = modalManager; }
    void refreshModList();
    void rescanConflicts();
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
    void activateSearch();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

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
    void onItchUpdateFound(const QString &modId, const ItchUpdateInfo &updateInfo);
    void onItchUpdateCheckComplete(int updatesFound);
    void onFilesDropped(const QStringList &filePaths);
    void onConflictScanComplete(QMap<QString, ConflictInfo> conflicts);
    void onConflictDetailsRequested(const QString &modId);
    void onRegisterWithNexusRequested(const QString &modId);
    void onRegisterWithItchRequested(const QString &modId);

private:
    void setupUi();
    QString getSelectedModId() const;
    int getSelectedRow() const;
    QString extractVersionFromFilename(const QString &filename) const;
    void handlePakFile(const QString &pakPath);
    void handleArchiveFile(const QString &archivePath);
    bool isArchiveFile(const QString &filePath) const;
    void hideSearch(bool clearFilter);
    void updateDragMode();
    bool isFilterActive() const;

    ModManager *m_modManager = nullptr;
    NexusModsClient *m_nexusClient = nullptr;
    NexusModsAuth *m_nexusAuth = nullptr;
    ItchClient *m_itchClient = nullptr;
    ItchAuth *m_itchAuth = nullptr;
    ModUpdateService *m_updateService = nullptr;
    ItchModUpdateService *m_itchUpdateService = nullptr;
    ModalManager *m_modalManager = nullptr;
    ModConflictDetector *m_conflictDetector = nullptr;
    DraggableModList *m_modList;

    QLabel *m_loadingLabel;
    QLabel *m_modCountLabel;
    QPushButton *m_checkUpdatesButton;
    QTimer *m_loadingTimer;
    QWidget *m_searchContainer = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QString m_filterText;
    int m_loadingDots = 0;

    bool m_updating = false;
    int m_pendingUpdateChecks = 0;
    int m_totalUpdatesFound = 0;
};

#endif // MODLISTWIDGET_H
