#include "NexusFileSelectionDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <algorithm>

NexusFileSelectionDialog::NexusFileSelectionDialog(const QList<NexusFileInfo> &files,
                                                   const QString &modName,
                                                   QWidget *parent)
    : QDialog(parent)
    , m_fileList(new QListWidget(this))
    , m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
    m_files = filterAndSortFiles(files);
    setupUi(m_files, modName);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void NexusFileSelectionDialog::setupUi(const QList<NexusFileInfo> &files, const QString &modName) {
    setWindowTitle(QStringLiteral("Select File - ") + modName);
    setMinimumWidth(600);
    setMinimumHeight(400);

    auto *layout = new QVBoxLayout(this);

    auto *headerLabel = new QLabel(QStringLiteral("Multiple files found. Please select one:"), this);
    layout->addWidget(headerLabel);

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

    for (const NexusFileInfo &file : files) {
        QString displayText = file.name;

        if (!file.version.isEmpty()) {
            displayText += QStringLiteral(" (v") + file.version + QStringLiteral(")");
        }

        if (file.sizeBytes > 0) {
            displayText += QStringLiteral(" - ") + formatFileSize(file.sizeBytes);
        }

        if (!file.categoryName.isEmpty()) {
            displayText += QStringLiteral(" [") + file.categoryName + QStringLiteral("]");
        }

        m_fileList->addItem(displayText);
    }

    if (!files.isEmpty()) {
        m_fileList->setCurrentRow(0);
    }

    layout->addWidget(m_fileList);
    layout->addWidget(m_buttonBox);
}

QString NexusFileSelectionDialog::getSelectedFileId() const {
    int row = m_fileList->currentRow();
    if (row >= 0 && row < m_files.size()) {
        return m_files[row].id;
    }
    return QString();
}

QString NexusFileSelectionDialog::formatFileSize(qint64 bytes) const {
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

QList<NexusFileInfo> NexusFileSelectionDialog::filterAndSortFiles(const QList<NexusFileInfo> &files) const {
    QList<NexusFileInfo> filtered;

    for (const NexusFileInfo &file : files) {
        QString category = file.categoryName.toUpper();
        if (category != QStringLiteral("ARCHIVED")) {
            filtered.append(file);
        }
    }

    std::sort(filtered.begin(), filtered.end(), [this](const NexusFileInfo &a, const NexusFileInfo &b) {
        int priorityA = getCategoryPriority(a.categoryName);
        int priorityB = getCategoryPriority(b.categoryName);

        if (priorityA != priorityB) {
            return priorityA < priorityB;
        }

        return a.uploadedTime > b.uploadedTime;
    });

    return filtered;
}

int NexusFileSelectionDialog::getCategoryPriority(const QString &categoryName) const {
    QString category = categoryName.toUpper();

    if (category == QStringLiteral("MAIN")) return 1;
    if (category == QStringLiteral("UPDATE")) return 2;
    if (category == QStringLiteral("OPTIONAL")) return 3;
    if (category == QStringLiteral("MISCELLANEOUS")) return 4;
    if (category == QStringLiteral("OLD_VERSION")) return 5;

    return 6;
}
