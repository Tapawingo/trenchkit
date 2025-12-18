#include "TitleBar.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QWindow>
#include <QSizePolicy>

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
    , m_iconLabel(new QLabel(this))
    , m_titleLabel(new QLabel(this))
    , m_minimizeButton(new QPushButton(this))
    , m_closeButton(new QPushButton(this))
    , m_layout(new QHBoxLayout(this))
{
    setupUi();
    setupConnections();
}

void TitleBar::setupUi() {
    setObjectName("titleBar");

    QPixmap toolboxPixmap(":/logo_toolbox.png");
    int toolboxHeight = 38;
    int toolboxWidth = toolboxPixmap.width() * toolboxHeight / toolboxPixmap.height();
    m_iconLabel->setPixmap(toolboxPixmap.scaled(toolboxWidth, toolboxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_iconLabel->setFixedSize(toolboxWidth, toolboxHeight);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setObjectName("titleBarIcon");

    QPixmap titlePixmap(":/logo_title.png");
    int titleHeight = 38;
    int titleWidth = titlePixmap.width() * titleHeight / titlePixmap.height();
    m_titleLabel->setPixmap(titlePixmap.scaled(titleWidth, titleHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_titleLabel->setFixedSize(titleWidth, titleHeight);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setObjectName("titleBarTitle");

    m_minimizeButton->setText("−");
    m_minimizeButton->setFixedSize(64, 64);
    m_minimizeButton->setObjectName("minimizeButton");
    m_minimizeButton->setFocusPolicy(Qt::NoFocus);

    m_closeButton->setText("×");
    m_closeButton->setFixedSize(64, 64);
    m_closeButton->setObjectName("closeButton");
    m_closeButton->setFocusPolicy(Qt::NoFocus);

    m_layout->setContentsMargins(12, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setAlignment(Qt::AlignVCenter);
    m_layout->addWidget(m_iconLabel);
    m_layout->addSpacing(12);
    m_layout->addWidget(m_titleLabel);
    m_layout->addStretch();
    m_layout->addWidget(m_minimizeButton);
    m_layout->addWidget(m_closeButton);

    setLayout(m_layout);

    // Apply basic styling
    setStyleSheet(R"(
        #titleBar {
            background-color: #2c2c2c;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
        }
        #minimizeButton, #closeButton {
            background-color: transparent;
            color: #ffffff;
            border: none;
            font-size: 24px;
            font-weight: 300;
        }
        #minimizeButton:hover {
            background-color: #404040;
        }
        #closeButton:hover {
            background-color: #e81123;
        }
    )");
}

void TitleBar::setupConnections() {
    connect(m_minimizeButton, &QPushButton::clicked, this, &TitleBar::minimizeClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &TitleBar::closeClicked);
}

void TitleBar::setTitle(const QString &title) {
    Q_UNUSED(title);
}

void TitleBar::setIcon(const QIcon &icon) {
    Q_UNUSED(icon);
}

void TitleBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - window()->frameGeometry().topLeft();
        event->accept();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        window()->move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}
