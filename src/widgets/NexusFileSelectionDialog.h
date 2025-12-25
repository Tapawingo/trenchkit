#pragma once

#include <QDialog>
#include <QList>
#include "../utils/NexusFileInfo.h"

class QListWidget;
class QDialogButtonBox;
class QLabel;

class NexusFileSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit NexusFileSelectionDialog(const QList<NexusFileInfo> &files,
                                     const QString &modName,
                                     QWidget *parent = nullptr);

    QString getSelectedFileId() const;

private:
    void setupUi(const QList<NexusFileInfo> &files, const QString &modName);
    QString formatFileSize(qint64 bytes) const;
    QList<NexusFileInfo> filterAndSortFiles(const QList<NexusFileInfo> &files) const;
    int getCategoryPriority(const QString &categoryName) const;

    QListWidget *m_fileList;
    QDialogButtonBox *m_buttonBox;
    QList<NexusFileInfo> m_files;
};
