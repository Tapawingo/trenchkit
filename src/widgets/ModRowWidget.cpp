#include "ModRowWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>

ModRowWidget::ModRowWidget(const ModInfo &mod, QWidget *parent)
    : QWidget(parent)
    , m_modId(mod.id)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi(mod);
}

void ModRowWidget::setupUi(const ModInfo &mod) {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(4);

    auto *topLayout = new QHBoxLayout();
    topLayout->setSpacing(8);

    m_enabledCheckBox = new QCheckBox(this);
    m_enabledCheckBox->setChecked(mod.enabled);

    m_nameLabel = new QLabel(mod.name, this);
    m_nameLabel->setStyleSheet(R"(
        QLabel {
            font-size: 14px;
            font-weight: bold;
            color: #e0e0e0;
        }
    )");

    topLayout->addWidget(m_enabledCheckBox);
    topLayout->addWidget(m_nameLabel, 1);

    mainLayout->addLayout(topLayout);

    m_dateLabel = new QLabel(this);
    m_dateLabel->setText("Installed: " + mod.installDate.toString("yyyy-MM-dd hh:mm"));
    m_dateLabel->setStyleSheet(R"(
        QLabel {
            font-size: 11px;
            color: #888888;
            margin-left: 28px;
        }
    )");

    mainLayout->addWidget(m_dateLabel);

    setStyleSheet(R"(
        ModRowWidget {
            background-color: #2d2d2d;
            border: 1px solid #3d3d3d;
            border-radius: 4px;
        }
        ModRowWidget[selected="true"] {
            background-color: #0e639c;
            border: 2px solid #1177bb;
        }
        ModRowWidget:hover {
            background-color: #888;
            border-color: #4d4d4d;
        }
        ModRowWidget[selected="true"]:hover {
            background-color: #1177bb;
            border-color: #1a8dd8;
        }
    )");

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
