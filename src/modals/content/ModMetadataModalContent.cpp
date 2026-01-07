#include "ModMetadataModalContent.h"
#include "../../utils/Theme.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

ModMetadataModalContent::ModMetadataModalContent(const ModInfo &mod, QWidget *parent)
    : BaseModalContent(parent)
    , m_modId(mod.id)
    , m_priority(mod.priority)
    , m_enabled(mod.enabled)
    , m_numberedFileName(mod.numberedFileName)
{
    setTitle("Edit Mod Metadata");
    setupUi(mod);
    setPreferredSize(QSize(500, 750));
}

void ModMetadataModalContent::setupUi(const ModInfo &mod) {
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *scrollContent = new QWidget();
    auto *formLayout = new QFormLayout(scrollContent);
    formLayout->setVerticalSpacing(16);
    formLayout->setHorizontalSpacing(12);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->setContentsMargins(0, 0, 0, 0);

    m_nameEdit = new QLineEdit(mod.name, scrollContent);
    m_nameEdit->setMinimumHeight(32);
    formLayout->addRow("Name:", m_nameEdit);

    m_fileNameEdit = new QLineEdit(mod.fileName, scrollContent);
    m_fileNameEdit->setMinimumHeight(32);
    formLayout->addRow("File Name:", m_fileNameEdit);

    m_descriptionEdit = new QTextEdit(mod.description, scrollContent);
    m_descriptionEdit->setMinimumHeight(100);
    m_descriptionEdit->setMaximumHeight(100);
    m_descriptionEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow("Description:", m_descriptionEdit);

    m_nexusModIdEdit = new QLineEdit(mod.nexusModId, scrollContent);
    m_nexusModIdEdit->setMinimumHeight(32);
    m_nexusModIdEdit->setPlaceholderText("e.g., 12345");
    formLayout->addRow("Nexusmods Mod ID:", m_nexusModIdEdit);

    m_nexusFileIdEdit = new QLineEdit(mod.nexusFileId, scrollContent);
    m_nexusFileIdEdit->setMinimumHeight(32);
    m_nexusFileIdEdit->setPlaceholderText("e.g., 12345");
    formLayout->addRow("Nexusmods File ID:", m_nexusFileIdEdit);

    m_itchGameIdEdit = new QLineEdit(mod.itchGameId, scrollContent);
    m_itchGameIdEdit->setMinimumHeight(32);
    m_itchGameIdEdit->setPlaceholderText("e.g., 1276966");
    formLayout->addRow("Itch.io Game ID:", m_itchGameIdEdit);

    m_versionEdit = new QLineEdit(mod.version, scrollContent);
    m_versionEdit->setMinimumHeight(32);
    m_versionEdit->setPlaceholderText("e.g., 1.0.0");
    formLayout->addRow("Version:", m_versionEdit);

    m_authorEdit = new QLineEdit(mod.author, scrollContent);
    m_authorEdit->setMinimumHeight(32);
    formLayout->addRow("Author:", m_authorEdit);

    m_installDateEdit = new QDateTimeEdit(mod.installDate, scrollContent);
    m_installDateEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    m_installDateEdit->setCalendarPopup(true);
    formLayout->addRow("Install Date:", m_installDateEdit);

    m_uploadDateEdit = new QDateTimeEdit(mod.uploadDate.isValid() ? mod.uploadDate : QDateTime::currentDateTime(), scrollContent);
    m_uploadDateEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    m_uploadDateEdit->setCalendarPopup(true);
    m_uploadDateEdit->setSpecialValueText("Not Set");
    m_uploadDateEdit->setMinimumDateTime(QDateTime());
    formLayout->addRow("Upload Date:", m_uploadDateEdit);

    scrollArea->setWidget(scrollContent);
    bodyLayout()->addWidget(scrollArea);

    auto *okButton = new QPushButton("OK", this);
    okButton->setCursor(Qt::PointingHandCursor);
    connect(okButton, &QPushButton::clicked, this, &ModMetadataModalContent::accept);
    footerLayout()->addWidget(okButton);

    auto *cancelButton = new QPushButton("Cancel", this);
    cancelButton->setCursor(Qt::PointingHandCursor);
    connect(cancelButton, &QPushButton::clicked, this, &ModMetadataModalContent::reject);
    footerLayout()->addWidget(cancelButton);
}

ModInfo ModMetadataModalContent::getModInfo() const {
    ModInfo mod;
    mod.id = m_modId;
    mod.name = m_nameEdit->text();
    mod.fileName = m_fileNameEdit->text();
    mod.description = m_descriptionEdit->toPlainText();
    mod.nexusModId = m_nexusModIdEdit->text();
    mod.nexusFileId = m_nexusFileIdEdit->text();
    mod.itchGameId = m_itchGameIdEdit->text();
    mod.version = m_versionEdit->text();
    mod.author = m_authorEdit->text();
    mod.installDate = m_installDateEdit->dateTime();
    mod.uploadDate = m_uploadDateEdit->dateTime();
    mod.priority = m_priority;
    mod.enabled = m_enabled;
    mod.numberedFileName = m_numberedFileName;
    return mod;
}
