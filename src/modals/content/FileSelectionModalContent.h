#ifndef FILESELECTIONMODALCONTENT_H
#define FILESELECTIONMODALCONTENT_H

#include "../BaseModalContent.h"
#include <QString>
#include <QStringList>

class QListWidget;
class QLabel;
class QPushButton;

class FileSelectionModalContent : public BaseModalContent {
    Q_OBJECT

public:
    explicit FileSelectionModalContent(const QStringList &pakFiles,
                                      const QString &archiveName,
                                      bool multiSelect = false,
                                      QWidget *parent = nullptr);

    QString getSelectedFile() const { return m_selectedFile; }
    QStringList getSelectedFiles() const { return m_selectedFiles; }

public slots:
    void accept() override;

private:
    void setupUi(const QStringList &pakFiles, const QString &archiveName);

    QListWidget *m_fileList;
    QPushButton *m_okButton;
    QString m_selectedFile;
    QStringList m_selectedFiles;
    bool m_multiSelect;
};

#endif // FILESELECTIONMODALCONTENT_H
