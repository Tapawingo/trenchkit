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
    setFixedHeight(32);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setObjectName("titleBar");

    // Icon label
    m_iconLabel->setFixedSize(16, 16);
    m_iconLabel->setScaledContents(true);
    m_iconLabel->setObjectName("titleBarIcon");

    // Title label
    m_titleLabel->setObjectName("titleBarTitle");
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Window control buttons
    m_minimizeButton->setFixedSize(32, 32);
    m_minimizeButton->setText("−");
    m_minimizeButton->setObjectName("minimizeButton");
    m_minimizeButton->setFocusPolicy(Qt::NoFocus);

    m_closeButton->setFixedSize(32, 32);
    m_closeButton->setText("×");
    m_closeButton->setObjectName("closeButton");
    m_closeButton->setFocusPolicy(Qt::NoFocus);

    // Layout
    m_layout->setContentsMargins(8, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_iconLabel);
    m_layout->addSpacing(8);
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
        #titleBarTitle {
            color: #ffffff;
            font-size: 13px;
            font-weight: 500;
        }
        #minimizeButton, #closeButton {
            background-color: transparent;
            color: #ffffff;
            border: none;
            font-size: 16px;
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
    m_titleLabel->setText(title);
}

void TitleBar::setIcon(const QIcon &icon) {
    QPixmap pixmap = icon.pixmap(16, 16);
    m_iconLabel->setPixmap(pixmap);
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
