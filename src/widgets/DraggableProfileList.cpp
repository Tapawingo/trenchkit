#include "DraggableProfileList.h"
#include <QDropEvent>

DraggableProfileList::DraggableProfileList(QWidget *parent)
    : QListWidget(parent)
{
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDropIndicatorShown(true);
}

void DraggableProfileList::dropEvent(QDropEvent *event) {
    QListWidget::dropEvent(event);

    if (event->isAccepted()) {
        emit itemsReordered();
    }
}
