#include "ConflictResolutionModalContent.h"
#include "core/utils/Theme.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

ConflictResolutionModalContent::ConflictResolutionModalContent(const ModInfo &incoming,
                                                             const ModInfo &existing,
                                                             bool checksumMatch,
                                                             QWidget *parent)
    : BaseModalContent(parent)
    , m_incoming(incoming)
    , m_existing(existing)
    , m_checksumMatch(checksumMatch)
{
    setTitle("Mod Conflict");
    setupUi();
    setPreferredSize(QSize(520, 240));
}

void ConflictResolutionModalContent::setupUi() {
    auto *label = new QLabel(buildMessage(), this);
    label->setWordWrap(true);
    label->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                            .arg(Theme::Colors::TEXT_SECONDARY));
    bodyLayout()->addWidget(label);

    auto *ignoreButton = new QPushButton("Ignore", this);
    auto *overwriteButton = new QPushButton("Overwrite", this);
    auto *duplicateButton = new QPushButton("Duplicate", this);

    ignoreButton->setCursor(Qt::PointingHandCursor);
    overwriteButton->setCursor(Qt::PointingHandCursor);
    duplicateButton->setCursor(Qt::PointingHandCursor);

    connect(ignoreButton, &QPushButton::clicked, this, [this]() {
        m_action = Action::Ignore;
        reject();
    });
    connect(overwriteButton, &QPushButton::clicked, this, [this]() {
        m_action = Action::Overwrite;
        accept();
    });
    connect(duplicateButton, &QPushButton::clicked, this, [this]() {
        m_action = Action::Duplicate;
        accept();
    });

    footerLayout()->addWidget(ignoreButton);
    footerLayout()->addWidget(overwriteButton);
    footerLayout()->addWidget(duplicateButton);
}

QString ConflictResolutionModalContent::buildMessage() const {
    QString incomingLabel = m_incoming.name.isEmpty() ? m_incoming.fileName : m_incoming.name;
    QString existingLabel = m_existing.name.isEmpty() ? m_existing.fileName : m_existing.name;

    QString message = QString("The mod \"%1\" conflicts with an existing mod \"%2\".\n")
        .arg(incomingLabel, existingLabel);

    if (m_checksumMatch) {
        message += "The files appear to be identical.\n";
    }

    message += "\nChoose how to proceed:";
    return message;
}
