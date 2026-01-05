#pragma once

#include <QDialog>
#include <QList>
#include "../utils/ItchUploadInfo.h"

class QListWidget;
class QDialogButtonBox;
class QLabel;

class ItchFileSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ItchFileSelectionDialog(const QList<ItchUploadInfo> &uploads,
                                    const QString &gameName,
                                    QWidget *parent = nullptr);

    QStringList getSelectedUploadIds() const;

private:
    void setupUi(const QList<ItchUploadInfo> &uploads, const QString &gameName);
    QString formatFileSize(qint64 bytes) const;
    QList<ItchUploadInfo> sortUploads(const QList<ItchUploadInfo> &uploads) const;

    QListWidget *m_uploadList;
    QDialogButtonBox *m_buttonBox;
    QList<ItchUploadInfo> m_uploads;
};
