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
{
    setTitle("Select .pak File");
    setupUi(pakFiles, archiveName);
    setPreferredSize(QSize(500, 400));
}

void FileSelectionModalContent::setupUi(const QStringList &pakFiles, const QString &archiveName) {
    QString messageText;
    if (m_multiSelect) {
        messageText = QString(
            "The archive '%1' contains multiple .pak files. Select one or more (Ctrl+Click):"
        ).arg(archiveName);
    } else {
        messageText = QString(
            "The archive '%1' contains multiple .pak files. Select one:"
        ).arg(archiveName);
    }

    auto *messageLabel = new QLabel(messageText, this);
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                               .arg(Theme::Colors::TEXT_SECONDARY));
    bodyLayout()->addWidget(messageLabel);

    m_fileList = new QListWidget(this);
    if (m_multiSelect) {
        m_fileList->setSelectionMode(QAbstractItemView::MultiSelection);
    } else {
        m_fileList->setSelectionMode(QAbstractItemView::SingleSelection);
    }
    m_fileList->addItems(pakFiles);
    if (!pakFiles.isEmpty()) {
        m_fileList->setCurrentRow(0);
    }
    bodyLayout()->addWidget(m_fileList);

    m_okButton = new QPushButton("OK", this);
    connect(m_okButton, &QPushButton::clicked, this, &FileSelectionModalContent::accept);
    footerLayout()->addWidget(m_okButton);

    auto *cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &FileSelectionModalContent::reject);
    footerLayout()->addWidget(cancelButton);

    connect(m_fileList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        if (!m_multiSelect) {
            m_selectedFile = item->text();
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

void FileSelectionModalContent::accept() {
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
