#include "FileSelectionDialog.h"
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

FileSelectionDialog::FileSelectionDialog(
    const QStringList &pakFiles,
    const QString &archiveName,
    bool multiSelect,
    QWidget *parent
)
    : QDialog(parent)
    , m_fileList(new QListWidget(this))
    , m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
    , m_multiSelect(multiSelect)
{
    setupUi(pakFiles, archiveName);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (m_multiSelect) {
            auto selectedItems = m_fileList->selectedItems();
            m_selectedFiles.clear();
            for (auto *item : selectedItems) {
                m_selectedFiles.append(item->text());
            }
            if (!m_selectedFiles.isEmpty()) {
                accept();
            }
        } else {
            auto *item = m_fileList->currentItem();
            if (item) {
                m_selectedFile = item->text();
                accept();
            }
        }
    });

    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_fileList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        if (!m_multiSelect) {
            m_selectedFile = item->text();
            accept();
        }
    });

    if (m_multiSelect) {
        connect(m_fileList, &QListWidget::itemSelectionChanged, this, [this]() {
            m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
                !m_fileList->selectedItems().isEmpty()
            );
        });
    } else {
        connect(m_fileList, &QListWidget::currentRowChanged, this, [this](int row) {
            m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(row >= 0);
        });
    }
}

QString FileSelectionDialog::getSelectedFile() const {
    return m_selectedFile;
}

QStringList FileSelectionDialog::getSelectedFiles() const {
    return m_selectedFiles;
}

void FileSelectionDialog::setupUi(const QStringList &pakFiles, const QString &archiveName) {
    setWindowTitle("Select .pak File");
    setMinimumWidth(500);
    setMinimumHeight(300);

    auto *layout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel("Multiple .pak files found", this);
    titleLabel->setObjectName("dialogTitle");
    layout->addWidget(titleLabel);

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
    layout->addWidget(messageLabel);

    if (m_multiSelect) {
        m_fileList->setSelectionMode(QAbstractItemView::MultiSelection);
    } else {
        m_fileList->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    m_fileList->addItems(pakFiles);
    if (!pakFiles.isEmpty()) {
        m_fileList->setCurrentRow(0);
    }
    layout->addWidget(m_fileList);

    layout->addWidget(m_buttonBox);

    setLayout(layout);
}
