#ifndef MODMANAGER_H
#define MODMANAGER_H

#include "ModInfo.h"
#include <QObject>
#include <QList>
#include <QString>

class ModManager : public QObject {
    Q_OBJECT

public:
    explicit ModManager(QObject *parent = nullptr);
    ~ModManager() override = default;

    // Initialization
    void setInstallPath(const QString &foxholeInstallPath);
    void setModsStoragePath(const QString &modsPath);

    // Mod management
    bool addMod(const QString &pakFilePath, const QString &modName = QString());
    bool removeMod(const QString &modId);
    bool enableMod(const QString &modId);
    bool disableMod(const QString &modId);
    bool setModPriority(const QString &modId, int priority);

    // Getters
    QList<ModInfo> getMods() const;
    ModInfo getMod(const QString &modId) const;
    QString getModsStoragePath() const { return m_modsStoragePath; }
    QString getPaksPath() const;

    // Persistence
    bool loadMods();
    bool saveMods();

    // Unregistered mod detection
    void detectUnregisteredMods();
    void syncEnabledModsWithPaks();

signals:
    void modsChanged();
    void modEnabled(const QString &modId);
    void modDisabled(const QString &modId);
    void modAdded(const QString &modId);
    void modRemoved(const QString &modId);
    void errorOccurred(const QString &error);

private:
    QString getModFilePath(const QString &modId) const;
    QString getMetadataFilePath() const;
    bool copyModToPaks(const ModInfo &mod);
    bool removeModFromPaks(const ModInfo &mod);
    void sortModsByPriority();
    QString generateNumberedFileName(int priority, const QString &originalFileName) const;
    void updateNumberedFileNames();
    void renumberEnabledMods();
    QString cleanModName(const QString &fileName) const;
    bool isBaseGamePak(const QString &fileName) const;

    QString m_foxholeInstallPath;
    QString m_modsStoragePath;
    QList<ModInfo> m_mods;
};

#endif // MODMANAGER_H
