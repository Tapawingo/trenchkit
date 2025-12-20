#include "ModRowWidget.h"
#include "../utils/Theme.h"
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
    setupUi(mod);
}

void ModRowWidget::setupUi(const ModInfo &mod) {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(
        Theme::Spacing::MOD_ROW_PADDING_HORIZONTAL,
        Theme::Spacing::MOD_ROW_PADDING_VERTICAL,
        Theme::Spacing::MOD_ROW_PADDING_HORIZONTAL,
        Theme::Spacing::MOD_ROW_PADDING_VERTICAL
    );
    mainLayout->setSpacing(0);

    auto *topLayout = new QHBoxLayout();
    topLayout->setSpacing(Theme::Spacing::MOD_ROW_INTERNAL_SPACING);

    m_enabledCheckBox = new QCheckBox(this);
    m_enabledCheckBox->setChecked(mod.enabled);

    m_nameLabel = new QLabel(mod.name, this);
    m_nameLabel->setObjectName("modNameLabel");

    topLayout->addWidget(m_enabledCheckBox);
    topLayout->addWidget(m_nameLabel, 1);

    mainLayout->addLayout(topLayout);

    m_dateLabel = new QLabel(this);
    m_dateLabel->setText("Installed: " + mod.installDate.toString("yyyy-MM-dd hh:mm"));
    m_dateLabel->setObjectName("modDateLabel");

    mainLayout->addWidget(m_dateLabel);

    setProperty("selected", false);

    connect(m_enabledCheckBox, &QCheckBox::checkStateChanged,
            this, [this](Qt::CheckState state) {
        emit enabledChanged(m_modId, state == Qt::Checked);
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
