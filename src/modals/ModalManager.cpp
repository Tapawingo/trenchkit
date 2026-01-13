#include "ModalManager.h"
#include "ModalOverlay.h"
#include "BaseModalContent.h"
#include <QWidget>

ModalManager::ModalManager(QWidget *mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
{
}

void ModalManager::showModal(BaseModalContent *content) {
    if (!content || !m_mainWindow) return;

    auto *overlay = new ModalOverlay(content, m_mainWindow);
    connect(overlay, &ModalOverlay::closed, this, &ModalManager::onOverlayClosed);

    m_modalStack.append(overlay);
    overlay->show();

    emit modalOpened();
}

void ModalManager::closeCurrentModal() {
    if (m_modalStack.isEmpty()) return;

    auto *overlay = m_modalStack.last();
    if (overlay) {
        overlay->close();
    }
}

void ModalManager::closeAllModals() {
    while (!m_modalStack.isEmpty()) {
        auto *overlay = m_modalStack.takeLast();
        if (overlay) {
            overlay->close();
        }
    }
}

bool ModalManager::hasOpenModal() const {
    return !m_modalStack.isEmpty();
}

void ModalManager::onOverlayClosed(int result) {
    auto *overlay = qobject_cast<ModalOverlay*>(sender());
    if (overlay) {
        m_modalStack.removeOne(overlay);
    }

    emit modalClosed(result);
}
