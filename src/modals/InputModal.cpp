#include "InputModal.h"
#include "ModalManager.h"
#include "../utils/Theme.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

InputModal::InputModal(const QString &title,
                      const QString &label,
                      const QString &defaultValue,
                      QWidget *parent)
    : BaseModalContent(parent)
{
    setTitle(title);
    setupUi(label, defaultValue);
    setPreferredSize(QSize(400, 180));
}

void InputModal::setupUi(const QString &label, const QString &defaultValue) {
    auto *labelWidget = new QLabel(label, this);
    labelWidget->setStyleSheet(QString("QLabel { color: %1; font-size: 13px; }")
                              .arg(Theme::Colors::TEXT_SECONDARY));
    bodyLayout()->addWidget(labelWidget);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setText(defaultValue);
    bodyLayout()->addWidget(m_lineEdit);

    auto *okButton = new QPushButton("OK", this);
    connect(okButton, &QPushButton::clicked, this, &InputModal::accept);
    footerLayout()->addWidget(okButton);

    auto *cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &InputModal::reject);
    footerLayout()->addWidget(cancelButton);
}

QString InputModal::textValue() const {
    return m_lineEdit->text();
}

QString InputModal::getText(ModalManager *manager,
                           const QString &title,
                           const QString &label,
                           const QString &defaultValue,
                           bool *ok) {
    if (!manager) {
        if (ok) *ok = false;
        return QString();
    }

    QString result;
    bool accepted = false;

    auto *modal = new InputModal(title, label, defaultValue);

    QObject::connect(modal, &InputModal::accepted, [&result, &accepted, modal]() {
        result = modal->textValue();
        accepted = true;
    });

    if (ok) *ok = accepted;
    manager->showModal(modal);

    return result;
}
