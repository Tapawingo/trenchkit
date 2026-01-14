#ifndef DRAGGABLEPROFILELIST_H
#define DRAGGABLEPROFILELIST_H

#include <QListWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>

class DraggableProfileList : public QListWidget {
    Q_OBJECT

public:
    explicit DraggableProfileList(QWidget *parent = nullptr);
    ~DraggableProfileList() override = default;

signals:
    void itemsReordered();

protected:
    void dropEvent(QDropEvent *event) override;
};

#endif // DRAGGABLEPROFILELIST_H
