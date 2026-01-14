#include "ActionsWidget.h"
#include "common/widgets/GradientFrame.h"
#include "core/utils/Theme.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>

ActionsWidget::ActionsWidget(QWidget *parent)
    : QWidget(parent)
    , m_addButton(new QPushButton("Add Mod", this))
    , m_removeButton(new QPushButton("Remove Mod", this))
    , m_moveUpButton(new QPushButton("Move Up", this))
    , m_moveDownButton(new QPushButton("Move Down", this))
    , m_exploreFolderButton(new QPushButton("Explore Mod Folder", this))
{
    setupUi();
    setupConnections();
}

void ActionsWidget::setFoxholeInstallPath(const QString &path) {
    m_foxholeInstallPath = path;
}

void ActionsWidget::onModSelectionChanged(int selectedRow, int totalMods) {
    m_selectedRow = selectedRow;
    m_totalMods = totalMods;

    bool hasSelection = selectedRow >= 0;
    bool isFirstRow = selectedRow == 0;
    bool isLastRow = selectedRow == totalMods - 1;

    m_removeButton->setEnabled(hasSelection);
    m_moveUpButton->setEnabled(hasSelection && !isFirstRow);
    m_moveDownButton->setEnabled(hasSelection && !isLastRow);
}

void ActionsWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN,
        Theme::Spacing::CONTAINER_MARGIN
    );
    layout->setSpacing(Theme::Spacing::CONTAINER_SPACING);

    GradientFrame *frame = new GradientFrame(this);
    layout->addWidget(frame);

    QVBoxLayout *frameLayout = new QVBoxLayout(frame);

    auto *titleLabel = new QLabel("Actions", this);
    titleLabel->setObjectName("modActionsTitle");
    frameLayout->addWidget(titleLabel);

    m_addButton->setCursor(Qt::PointingHandCursor);
    m_removeButton->setEnabled(false);
    m_removeButton->setCursor(Qt::PointingHandCursor);
    frameLayout->addWidget(m_addButton);
    frameLayout->addWidget(m_removeButton);

    m_moveUpButton->setEnabled(false);
    m_moveUpButton->setCursor(Qt::PointingHandCursor);
    frameLayout->addWidget(m_moveUpButton);

    m_moveDownButton->setEnabled(false);
    m_moveDownButton->setCursor(Qt::PointingHandCursor);
    frameLayout->addWidget(m_moveDownButton);

    frameLayout->addWidget(createSeparator());

    m_exploreFolderButton->setCursor(Qt::PointingHandCursor);
    frameLayout->addWidget(m_exploreFolderButton);

    setLayout(layout);
}

void ActionsWidget::setupConnections() {
    connect(m_addButton, &QPushButton::clicked, this, &ActionsWidget::onAddModClicked);
    connect(m_removeButton, &QPushButton::clicked, this, &ActionsWidget::onRemoveModClicked);
    connect(m_moveUpButton, &QPushButton::clicked, this, &ActionsWidget::onMoveUpClicked);
    connect(m_moveDownButton, &QPushButton::clicked, this, &ActionsWidget::onMoveDownClicked);
    connect(m_exploreFolderButton, &QPushButton::clicked, this, &ActionsWidget::onExploreFolderClicked);
}

QFrame* ActionsWidget::createSeparator() {
    QFrame *line = new QFrame(this);
    line->setObjectName("actionsSeparator");
    line->setFrameShape(QFrame::NoFrame);
    line->setFixedHeight(1);
    return line;
}

void ActionsWidget::onAddModClicked() {
    emit addModRequested();
}

void ActionsWidget::onRemoveModClicked() {
    emit removeModRequested();
}

void ActionsWidget::onMoveUpClicked() {
    emit moveUpRequested();
}

void ActionsWidget::onMoveDownClicked() {
    emit moveDownRequested();
}

void ActionsWidget::onExploreFolderClicked() {
    if (m_foxholeInstallPath.isEmpty()) {
        emit errorOccurred("Foxhole installation path not set");
        return;
    }

    QString paksPath = m_foxholeInstallPath + "/War/Content/Paks";
    QDir dir(paksPath);

    if (!dir.exists()) {
        emit errorOccurred("Paks folder not found: " + paksPath);
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(paksPath));
}
