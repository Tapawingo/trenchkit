#ifndef MODALMANAGER_H
#define MODALMANAGER_H

#include <QObject>
#include <QList>

class QWidget;
class BaseModalContent;
class ModalOverlay;

class ModalManager : public QObject {
    Q_OBJECT

public:
    explicit ModalManager(QWidget *mainWindow, QObject *parent = nullptr);
    ~ModalManager() override = default;

    void showModal(BaseModalContent *content);
    void closeCurrentModal();
    void closeAllModals();

signals:
    void modalOpened();
    void modalClosed(int result);

private slots:
    void onOverlayClosed(int result);

private:
    QWidget *m_mainWindow;
    QList<ModalOverlay*> m_modalStack;
};

#endif // MODALMANAGER_H
