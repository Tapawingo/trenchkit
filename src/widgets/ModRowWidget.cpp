#include "ModRowWidget.h"
#include "../utils/Theme.h"
#include "../utils/ModConflictDetector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QMenu>
#include <QContextMenuEvent>

ModRowWidget::ModRowWidget(const ModInfo &mod, QWidget *parent)
    : QWidget(parent)
    , m_modId(mod.id)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setCursor(Qt::PointingHandCursor);
    setupUi(mod);
}

void ModRowWidget::setupUi(const ModInfo &mod) {
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(
        Theme::Spacing::MOD_ROW_PADDING_HORIZONTAL,
        Theme::Spacing::MOD_ROW_PADDING_VERTICAL,
        3,
        Theme::Spacing::MOD_ROW_PADDING_VERTICAL
    );
    mainLayout->setSpacing(Theme::Spacing::MOD_ROW_INTERNAL_SPACING);

    auto *leftSection = new QVBoxLayout();
    leftSection->setSpacing(0);

    auto *topLayout = new QHBoxLayout();
    topLayout->setSpacing(Theme::Spacing::MOD_ROW_INTERNAL_SPACING);

    m_enabledCheckBox = new QCheckBox(this);
    m_enabledCheckBox->setChecked(mod.enabled);

    m_nameLabel = new QLabel(mod.name, this);
    m_nameLabel->setObjectName("modNameLabel");

    topLayout->addWidget(m_enabledCheckBox);
    topLayout->addWidget(m_nameLabel, 1);

    leftSection->addLayout(topLayout);

    m_dateLabel = new QLabel(this);
    m_dateLabel->setText("Installed: " + mod.installDate.toString("yyyy-MM-dd hh:mm"));
    m_dateLabel->setObjectName("modDateLabel");

    leftSection->addWidget(m_dateLabel);

    mainLayout->addLayout(leftSection, 1);

    m_updateButton = new QPushButton(this);
    m_updateButton->setObjectName("modUpdateButton");
    m_updateButton->setVisible(false);
    m_updateButton->setIcon(QIcon(":/icon_update.png"));
    m_updateButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_updateButton->setCursor(Qt::PointingHandCursor);

    mainLayout->addWidget(m_updateButton);

    m_conflictButton = new QPushButton(this);
    m_conflictButton->setObjectName("modConflictButton");
    m_conflictButton->setVisible(false);
    m_conflictButton->setIcon(QIcon(":/icon_warning.png"));
    m_conflictButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_conflictButton->setCursor(Qt::PointingHandCursor);
    m_conflictButton->setFlat(true);
    m_conflictButton->setFocusPolicy(Qt::NoFocus);

    connect(m_conflictButton, &QPushButton::clicked, this, [this]() {
        emit conflictDetailsRequested(m_modId);
    });

    mainLayout->addWidget(m_conflictButton);
    mainLayout->addSpacing(6);

    setProperty("selected", false);

    connect(m_enabledCheckBox, &QCheckBox::checkStateChanged,
            this, [this](Qt::CheckState state) {
        emit enabledChanged(m_modId, state == Qt::Checked);
    });

    connect(m_updateButton, &QPushButton::clicked, this, [this]() {
        emit updateRequested(m_modId);
    });
}

void ModRowWidget::updateModInfo(const ModInfo &mod) {
    m_enabledCheckBox->setChecked(mod.enabled);
    m_nameLabel->setText(mod.name);
    m_dateLabel->setText("Installed: " + mod.installDate.toString("yyyy-MM-dd hh:mm"));
}

void ModRowWidget::setSelected(bool selected) {
    if (m_selected != selected) {
        m_selected = selected;
        updateStyling();
    }
}

void ModRowWidget::updateStyling() {
    setProperty("selected", m_selected);
    style()->unpolish(this);
    style()->polish(this);
}

void ModRowWidget::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);

    QAction *renameAction = menu.addAction("Rename");
    QAction *editMetaAction = menu.addAction("Edit Metadata");
    menu.addSeparator();
    QAction *removeAction = menu.addAction("Remove");

    QAction *selectedAction = menu.exec(event->globalPos());

    if (selectedAction == renameAction) {
        emit renameRequested(m_modId);
    } else if (selectedAction == editMetaAction) {
        emit editMetaRequested(m_modId);
    } else if (selectedAction == removeAction) {
        emit removeRequested(m_modId);
    }
}

void ModRowWidget::setUpdateAvailable(bool available, const QString &version) {
    if (m_updateButton) {
        m_updateButton->setVisible(available);
        if (available) {
            if (!version.isEmpty()) {
                m_updateButton->setToolTip(QStringLiteral("Update to version %1").arg(version));
            }
            int buttonHeight = m_updateButton->height();
            if (buttonHeight > 0) {
                m_updateButton->setFixedWidth(buttonHeight);
            }
        }
    }
}

void ModRowWidget::hideUpdateButton() {
    if (m_updateButton) {
        m_updateButton->setVisible(false);
    }
}

void ModRowWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);

    if (m_updateButton && m_updateButton->isVisible()) {
        int buttonHeight = m_updateButton->height();
        m_updateButton->setFixedWidth(buttonHeight);
    }

    if (m_conflictButton && m_conflictButton->isVisible()) {
        int buttonHeight = m_conflictButton->height();
        m_conflictButton->setFixedWidth(buttonHeight);
    }
}

void ModRowWidget::setConflictInfo(const ConflictInfo &info) {
    if (!m_conflictButton) {
        return;
    }

    bool hasConflicts = info.hasConflicts();
    m_conflictButton->setVisible(hasConflicts);

    if (hasConflicts) {
        QString tooltip = formatConflictTooltip(info);
        m_conflictButton->setToolTip(tooltip);

        int buttonHeight = m_conflictButton->height();
        if (buttonHeight > 0) {
            m_conflictButton->setFixedWidth(buttonHeight);
        }
    }
}

void ModRowWidget::clearConflictIndicator() {
    if (m_conflictButton) {
        m_conflictButton->setVisible(false);
    }
}

QString ModRowWidget::formatConflictTooltip(const ConflictInfo &info) const {
    QString tooltip = QString("<b>⚠️ File Conflicts (%1 files)</b><br><br>")
        .arg(info.fileConflictCount);

    tooltip += "<b>Conflicts with:</b><br>";

    for (int i = 0; i < info.conflictingModNames.size(); ++i) {
        QString modName = info.conflictingModNames[i];
        int otherPriority = info.conflictingModPriorities[i];

        QString loadOrder;
        if (otherPriority < info.modPriority) {
            loadOrder = " <i>(loads before this)</i>";
        } else {
            loadOrder = " <i>(loads after this)</i>";
        }

        tooltip += QString("• %1%2<br>").arg(modName, loadOrder);
    }

    if (!info.conflictingFilePaths.isEmpty()) {
        tooltip += "<br><b>Sample conflicts:</b><br>";
        int sampleCount = qMin(5, info.conflictingFilePaths.size());
        for (int i = 0; i < sampleCount; ++i) {
            tooltip += QString("• %1<br>").arg(info.conflictingFilePaths[i]);
        }
        if (info.conflictingFilePaths.size() > 5) {
            tooltip += QString("• ... and %1 more<br>")
                .arg(info.conflictingFilePaths.size() - 5);
        }
    }

    return tooltip;
}
