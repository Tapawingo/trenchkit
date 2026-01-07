#include "FileSelectionModalContent.h"
#include "../../utils/Theme.h"
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

FileSelectionModalContent::FileSelectionModalContent(const QStringList &pakFiles,
                                                    const QString &archiveName,
                                                    bool multiSelect,
                                                    QWidget *parent)
    : BaseModalContent(parent)
    , m_multiSelect(multiSelect)
    , m_useFileItems(false)
{
    setTitle("Select .pak File");

    QString headerText;
    if (m_multiSelect) {
        headerText = QString(
            "The archive '%1' contains multiple .pak files. Select one or more (Ctrl+Click):"
        ).arg(archiveName);
    } else {
        headerText = QString(
            "The archive '%1' contains multiple .pak files. Select one:"
        ).arg(archiveName);
    }

    setupUi(headerText);
    m_fileList->addItems(pakFiles);
    if (!pakFiles.isEmpty()) {
        m_fileList->setCurrentRow(0);
    }

    setPreferredSize(QSize(500, 400));
}

FileSelectionModalContent::FileSelectionModalContent(const QList<FileItem> &items,
                                                    const QString &title,
                                                    const QString &headerText,
                                                    bool multiSelect,
                                                    QWidget *parent)
    : BaseModalContent(parent)
    , m_fileItems(items)
    , m_multiSelect(multiSelect)
    , m_useFileItems(true)
{
    setTitle(title);
    setupUi(headerText);

    for (const FileItem &item : m_fileItems) {
        m_fileList->addItem(item.displayText);
    }

    if (!m_fileItems.isEmpty()) {
        m_fileList->setCurrentRow(0);
    }

    setPreferredSize(QSize(600, 400));
}

void FileSelectionModalContent::setupUi(const QString &headerText) {
    auto *headerLabel = new QLabel(headerText, this);
    headerLabel->setWordWrap(true);
    headerLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                              .arg(Theme::Colors::TEXT_SECONDARY));
    bodyLayout()->addWidget(headerLabel);

    m_fileList = new QListWidget(this);
    if (m_multiSelect) {
        m_fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    } else {
        m_fileList->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    applyListStyling();
    bodyLayout()->addWidget(m_fileList);

    m_okButton = new QPushButton("OK", this);
    connect(m_okButton, &QPushButton::clicked, this, &FileSelectionModalContent::accept);
    footerLayout()->addWidget(m_okButton);

    auto *cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &FileSelectionModalContent::reject);
    footerLayout()->addWidget(cancelButton);

    connect(m_fileList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        Q_UNUSED(item);
        if (!m_multiSelect) {
            BaseModalContent::accept();
        }
    });

    if (m_multiSelect) {
        connect(m_fileList, &QListWidget::itemSelectionChanged, this, [this]() {
            m_okButton->setEnabled(!m_fileList->selectedItems().isEmpty());
        });
    } else {
        connect(m_fileList, &QListWidget::currentRowChanged, this, [this](int row) {
            m_okButton->setEnabled(row >= 0);
        });
    }
}

void FileSelectionModalContent::applyListStyling() {
    m_fileList->setStyleSheet(
        "QListWidget {"
        "    background-color: #2a2a2a;"
        "    border: 1px solid #3a3a3a;"
        "    border-radius: 4px;"
        "    padding: 4px;"
        "    color: #e0e0e0;"
        "}"
        "QListWidget::item {"
        "    padding: 8px;"
        "    border-radius: 3px;"
        "    margin: 2px 0px;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #3a3a3a;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #4a6fa5;"
        "    color: white;"
        "}"
        "QListWidget::item:selected:hover {"
        "    background-color: #5a7fb5;"
        "}"
    );
}

void FileSelectionModalContent::accept() {
    if (m_useFileItems) {
        auto selectedItems = m_fileList->selectedItems();
        m_selectedIds.clear();

        for (auto *item : selectedItems) {
            int row = m_fileList->row(item);
            if (row >= 0 && row < m_fileItems.size()) {
                m_selectedIds.append(m_fileItems[row].id);
            }
        }

        if (!m_selectedIds.isEmpty()) {
            BaseModalContent::accept();
        }
    } else {
        if (m_multiSelect) {
            auto selectedItems = m_fileList->selectedItems();
            m_selectedFiles.clear();
            for (auto *item : selectedItems) {
                m_selectedFiles.append(item->text());
            }
            if (!m_selectedFiles.isEmpty()) {
                BaseModalContent::accept();
            }
        } else {
            auto *item = m_fileList->currentItem();
            if (item) {
                m_selectedFile = item->text();
                BaseModalContent::accept();
            }
        }
    }
}
