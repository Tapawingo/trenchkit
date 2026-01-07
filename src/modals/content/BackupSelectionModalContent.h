#ifndef BACKUPSELECTIONMODALCONTENT_H
#define BACKUPSELECTIONMODALCONTENT_H

#include "../BaseModalContent.h"
#include <QStringList>

class QListWidget;

class BackupSelectionModalContent : public BaseModalContent {
    Q_OBJECT

public:
    explicit BackupSelectionModalContent(const QStringList &backups,
                                         const QStringList &displayNames,
                                         QWidget *parent = nullptr);

    QString getSelectedBackup() const;

private:
    void setupUi(const QStringList &displayNames);

    QStringList m_backups;
    QListWidget *m_listWidget;
};

#endif // BACKUPSELECTIONMODALCONTENT_H
