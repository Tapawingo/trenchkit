#include "MessageModal.h"
#include "ModalManager.h"
#include "../utils/Theme.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

MessageModal::MessageModal(const QString &title,
                          const QString &text,
                          Icon icon,
                          StandardButtons buttons,
                          QWidget *parent)
    : BaseModalContent(parent)
{
    setTitle(title);
    setupUi(text, icon, buttons);
    setPreferredSize(QSize(450, 200));
}

void MessageModal::setupUi(const QString &text, Icon icon, StandardButtons buttons) {
    auto *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(16);

    if (icon != NoIcon) {
        m_iconLabel = new QLabel(this);
        m_iconLabel->setText(getIconText(icon));
        m_iconLabel->setStyleSheet(QString("QLabel { font-size: 32px; }"));
        m_iconLabel->setFixedSize(40, 40);
        contentLayout->addWidget(m_iconLabel, 0, Qt::AlignTop);
    }

    m_textLabel = new QLabel(text, this);
    m_textLabel->setWordWrap(true);
    m_textLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                              .arg(Theme::Colors::TEXT_SECONDARY));
    contentLayout->addWidget(m_textLabel, 1);

    bodyLayout()->addLayout(contentLayout);

    if (buttons & Yes) {
        auto *yesButton = new QPushButton("Yes", this);
        yesButton->setCursor(Qt::PointingHandCursor);
        connect(yesButton, &QPushButton::clicked, this, [this]() {
            m_clickedButton = Yes;
            accept();
        });
        footerLayout()->addWidget(yesButton);
    }

    if (buttons & No) {
        auto *noButton = new QPushButton("No", this);
        noButton->setCursor(Qt::PointingHandCursor);
        connect(noButton, &QPushButton::clicked, this, [this]() {
            m_clickedButton = No;
            reject();
        });
        footerLayout()->addWidget(noButton);
    }

    if (buttons & Ok) {
        auto *okButton = new QPushButton("OK", this);
        okButton->setCursor(Qt::PointingHandCursor);
        connect(okButton, &QPushButton::clicked, this, [this]() {
            m_clickedButton = Ok;
            accept();
        });
        footerLayout()->addWidget(okButton);
    }

    if (buttons & Cancel) {
        auto *cancelButton = new QPushButton("Cancel", this);
        cancelButton->setCursor(Qt::PointingHandCursor);
        connect(cancelButton, &QPushButton::clicked, this, [this]() {
            m_clickedButton = Cancel;
            reject();
        });
        footerLayout()->addWidget(cancelButton);
    }
}

QString MessageModal::getIconText(Icon icon) {
    switch (icon) {
        case Information:
            return "ℹ️";
        case Warning:
            return "⚠️";
        case Critical:
            return "❌";
        case Question:
            return "❓";
        default:
            return "";
    }
}

void MessageModal::information(ModalManager *manager, const QString &title, const QString &text) {
    if (!manager) return;
    auto *modal = new MessageModal(title, text, Information, Ok);
    manager->showModal(modal);
}

void MessageModal::warning(ModalManager *manager, const QString &title, const QString &text) {
    if (!manager) return;
    auto *modal = new MessageModal(title, text, Warning, Ok);
    manager->showModal(modal);
}

void MessageModal::critical(ModalManager *manager, const QString &title, const QString &text) {
    if (!manager) return;
    auto *modal = new MessageModal(title, text, Critical, Ok);
    manager->showModal(modal);
}

bool MessageModal::question(ModalManager *manager, const QString &title, const QString &text) {
    if (!manager) return false;
    auto *modal = new MessageModal(title, text, Question, Yes | No);
    manager->showModal(modal);
    return true;
}
