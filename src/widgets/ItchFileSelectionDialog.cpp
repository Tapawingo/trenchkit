#include "ItchFileSelectionDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <algorithm>

ItchFileSelectionDialog::ItchFileSelectionDialog(const QList<ItchUploadInfo> &uploads,
                                                 const QString &gameName,
                                                 QWidget *parent)
    : QDialog(parent)
    , m_uploadList(new QListWidget(this))
    , m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    m_uploads = sortUploads(uploads);
    setupUi(m_uploads, gameName);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void ItchFileSelectionDialog::setupUi(const QList<ItchUploadInfo> &uploads, const QString &gameName) {
    setWindowTitle(QStringLiteral("Select Files - ") + gameName);
    setMinimumWidth(600);
    setMinimumHeight(400);

    auto *layout = new QVBoxLayout(this);

    auto *headerLabel = new QLabel(QStringLiteral("Multiple files found. Select one or more (Ctrl+Click or Shift+Click):"), this);
    layout->addWidget(headerLabel);

    // Enable multi-selection
    m_uploadList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_uploadList->setStyleSheet(
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

    for (const ItchUploadInfo &upload : uploads) {
        QString displayText = upload.filename;

        if (!upload.displayName.isEmpty()) {
            displayText = upload.displayName + QStringLiteral(" (") + upload.filename + QStringLiteral(")");
        }

        if (upload.sizeBytes > 0) {
            displayText += QStringLiteral(" - ") + formatFileSize(upload.sizeBytes);
        }

        if (upload.createdAt.isValid()) {
            displayText += QStringLiteral(" [") + upload.createdAt.toString(QStringLiteral("yyyy-MM-dd")) + QStringLiteral("]");
        }

        m_uploadList->addItem(displayText);
    }

    if (!uploads.isEmpty()) {
        m_uploadList->setCurrentRow(0);
    }

    layout->addWidget(m_uploadList);
    layout->addWidget(m_buttonBox);
}

QStringList ItchFileSelectionDialog::getSelectedUploadIds() const {
    QStringList ids;
    QList<QListWidgetItem*> selectedItems = m_uploadList->selectedItems();

    for (const QListWidgetItem *item : selectedItems) {
        int row = m_uploadList->row(item);
        if (row >= 0 && row < m_uploads.size()) {
            ids.append(m_uploads[row].id);
        }
    }

    return ids;
}

QString ItchFileSelectionDialog::formatFileSize(qint64 bytes) const {
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;

    if (bytes >= GB) {
        return QStringLiteral("%1 GB").arg(bytes / static_cast<double>(GB), 0, 'f', 2);
    } else if (bytes >= MB) {
        return QStringLiteral("%1 MB").arg(bytes / static_cast<double>(MB), 0, 'f', 2);
    } else if (bytes >= KB) {
        return QStringLiteral("%1 KB").arg(bytes / static_cast<double>(KB), 0, 'f', 2);
    } else {
        return QStringLiteral("%1 bytes").arg(bytes);
    }
}

QList<ItchUploadInfo> ItchFileSelectionDialog::sortUploads(const QList<ItchUploadInfo> &uploads) const {
    QList<ItchUploadInfo> sorted = uploads;

    // Sort by most recent first
    std::sort(sorted.begin(), sorted.end(), [](const ItchUploadInfo &a, const ItchUploadInfo &b) {
        return a.createdAt > b.createdAt;
    });

    return sorted;
}
