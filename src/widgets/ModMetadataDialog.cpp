#include "ModMetadataDialog.h"
#include "../utils/Theme.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QLabel>

ModMetadataDialog::ModMetadataDialog(const ModInfo &mod, QWidget *parent)
    : QDialog(parent)
    , m_modId(mod.id)
    , m_priority(mod.priority)
    , m_enabled(mod.enabled)
    , m_numberedFileName(mod.numberedFileName)
{
    setupUi(mod);
}

void ModMetadataDialog::setupUi(const ModInfo &mod) {
    setWindowTitle("Edit Mod Metadata");
    setMinimumWidth(500);

    auto *mainLayout = new QVBoxLayout(this);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(Theme::Spacing::FORM_SPACING);

    m_nameEdit = new QLineEdit(mod.name, this);
    formLayout->addRow("Name:", m_nameEdit);

    m_fileNameEdit = new QLineEdit(mod.fileName, this);
    formLayout->addRow("File Name:", m_fileNameEdit);

    m_descriptionEdit = new QTextEdit(mod.description, this);
    m_descriptionEdit->setMaximumHeight(100);
    formLayout->addRow("Description:", m_descriptionEdit);

    m_nexusModIdEdit = new QLineEdit(mod.nexusModId, this);
    m_nexusModIdEdit->setPlaceholderText("e.g., 12345");
    formLayout->addRow("Nexusmods Mod ID:", m_nexusModIdEdit);

    m_nexusFileIdEdit = new QLineEdit(mod.nexusFileId, this);
    m_nexusFileIdEdit->setPlaceholderText("e.g., 12345");
    formLayout->addRow("Nexusmods File ID:", m_nexusFileIdEdit);

    m_versionEdit = new QLineEdit(mod.version, this);
    m_versionEdit->setPlaceholderText("e.g., 1.0.0");
    formLayout->addRow("Version:", m_versionEdit);

    m_authorEdit = new QLineEdit(mod.author, this);
    formLayout->addRow("Author:", m_authorEdit);

    m_installDateEdit = new QDateTimeEdit(mod.installDate, this);
    m_installDateEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    m_installDateEdit->setCalendarPopup(true);
    formLayout->addRow("Install Date:", m_installDateEdit);

    mainLayout->addLayout(formLayout);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(m_buttonBox);
}

ModInfo ModMetadataDialog::getModInfo() const {
    ModInfo mod;
    mod.id = m_modId;
    mod.name = m_nameEdit->text();
    mod.fileName = m_fileNameEdit->text();
    mod.description = m_descriptionEdit->toPlainText();
    mod.nexusModId = m_nexusModIdEdit->text();
    mod.nexusFileId = m_nexusFileIdEdit->text();
    mod.version = m_versionEdit->text();
    mod.author = m_authorEdit->text();
    mod.installDate = m_installDateEdit->dateTime();
    mod.priority = m_priority;
    mod.enabled = m_enabled;
    mod.numberedFileName = m_numberedFileName;
    return mod;
}
