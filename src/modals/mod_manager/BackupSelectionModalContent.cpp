#include "BackupSelectionModalContent.h"
#include "core/utils/Theme.h"
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

BackupSelectionModalContent::BackupSelectionModalContent(const QStringList &backups,
                                                         const QStringList &displayNames,
                                                         QWidget *parent)
    : BaseModalContent(parent)
    , m_backups(backups)
{
    setTitle("Restore Backup");
    setupUi(displayNames);
    setPreferredSize(QSize(500, 400));
}

void BackupSelectionModalContent::setupUi(const QStringList &displayNames) {
    auto *label = new QLabel("Select a backup to restore:", this);
    label->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                        .arg(Theme::Colors::TEXT_SECONDARY));
    bodyLayout()->addWidget(label);

    m_listWidget = new QListWidget(this);
    m_listWidget->setStyleSheet(
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

    for (const QString &displayName : displayNames) {
        m_listWidget->addItem(displayName);
    }

    if (!displayNames.isEmpty()) {
        m_listWidget->setCurrentRow(0);
    }

    bodyLayout()->addWidget(m_listWidget);

    auto *okButton = new QPushButton("OK", this);
    okButton->setCursor(Qt::PointingHandCursor);
    connect(okButton, &QPushButton::clicked, this, &BackupSelectionModalContent::accept);
    footerLayout()->addWidget(okButton);

    auto *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setCursor(Qt::PointingHandCursor);
    connect(cancelButton, &QPushButton::clicked, this, &BackupSelectionModalContent::reject);
    footerLayout()->addWidget(cancelButton);

    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &BackupSelectionModalContent::accept);
}

QString BackupSelectionModalContent::getSelectedBackup() const {
    int index = m_listWidget->currentRow();
    if (index >= 0 && index < m_backups.size()) {
        return m_backups[index];
    }
    return QString();
}
