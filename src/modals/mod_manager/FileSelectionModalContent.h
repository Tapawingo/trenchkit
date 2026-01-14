#ifndef FILESELECTIONMODALCONTENT_H
#define FILESELECTIONMODALCONTENT_H

#include "common/modals/BaseModalContent.h"
#include <QString>
#include <QStringList>
#include <QList>

class QListWidget;
class QLabel;
class QPushButton;

struct FileItem {
    QString id;
    QString displayText;
};

class FileSelectionModalContent : public BaseModalContent {
    Q_OBJECT

public:
    explicit FileSelectionModalContent(const QStringList &pakFiles,
                                      const QString &archiveName,
                                      bool multiSelect = false,
                                      QWidget *parent = nullptr);

    explicit FileSelectionModalContent(const QList<FileItem> &items,
                                      const QString &title,
                                      const QString &headerText,
                                      bool multiSelect = false,
                                      bool showIgnoreButton = false,
                                      QWidget *parent = nullptr);

    QString getSelectedFile() const { return m_selectedFile; }
    QStringList getSelectedFiles() const { return m_selectedFiles; }
    QStringList getSelectedIds() const { return m_selectedIds; }

public slots:
    void accept() override;
    void ignoreAll();

signals:
    void allIgnored();

private:
    void setupUi(const QString &headerText);
    void applyListStyling();

    QListWidget *m_fileList;
    QPushButton *m_okButton;
    QPushButton *m_ignoreButton;
    QString m_selectedFile;
    QStringList m_selectedFiles;
    QStringList m_selectedIds;
    QList<FileItem> m_fileItems;
    bool m_multiSelect;
    bool m_useFileItems;
    bool m_showIgnoreButton;
};

#endif // FILESELECTIONMODALCONTENT_H
