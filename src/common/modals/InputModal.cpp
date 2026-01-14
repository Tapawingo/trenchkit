#include "InputModal.h"
#include "core/utils/Theme.h"
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

    auto *okButton = new QPushButton("OK", this);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &InputModal::accept);
    footerLayout()->addWidget(okButton);

    auto *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setAutoDefault(false);
    connect(cancelButton, &QPushButton::clicked, this, &InputModal::reject);
    footerLayout()->addWidget(cancelButton);
}

QString InputModal::textValue() const {
    return m_lineEdit->text();
}
