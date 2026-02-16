#include "InputModal.h"
#include "core/utils/Theme.h"
#include <QEvent>
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
    m_lineEdit->setMinimumHeight(32);
    connect(m_lineEdit, &QLineEdit::returnPressed, this, &InputModal::accept);
    bodyLayout()->addWidget(m_lineEdit);

    m_okButton = new QPushButton(tr("OK"), this);
    m_okButton->setDefault(true);
    connect(m_okButton, &QPushButton::clicked, this, &InputModal::accept);
    footerLayout()->addWidget(m_okButton);

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_cancelButton->setAutoDefault(false);
    connect(m_cancelButton, &QPushButton::clicked, this, &InputModal::reject);
    footerLayout()->addWidget(m_cancelButton);

    retranslateUi();
}

void InputModal::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    BaseModalContent::changeEvent(event);
}

void InputModal::retranslateUi() {
    m_okButton->setText(tr("OK"));
    m_cancelButton->setText(tr("Cancel"));
}

QString InputModal::textValue() const {
    return m_lineEdit->text();
}
