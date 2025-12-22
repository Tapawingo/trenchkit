#ifndef FILESELECTIONDIALOG_H
#define FILESELECTIONDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>

class QListWidget;
class QDialogButtonBox;
class QLabel;

class FileSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit FileSelectionDialog(
        const QStringList &pakFiles,
        const QString &archiveName,
        bool multiSelect = false,
        QWidget *parent = nullptr
    );

    QString getSelectedFile() const;
    QStringList getSelectedFiles() const;

private:
    void setupUi(const QStringList &pakFiles, const QString &archiveName);

    QListWidget *m_fileList;
    QDialogButtonBox *m_buttonBox;
    QString m_selectedFile;
    QStringList m_selectedFiles;
    bool m_multiSelect;
};

#endif // FILESELECTIONDIALOG_H
