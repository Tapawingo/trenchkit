#include "DraggableModList.h"
#include <QDropEvent>

DraggableModList::DraggableModList(QWidget *parent)
    : QListWidget(parent)
{
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDropIndicatorShown(true);
}

void DraggableModList::dropEvent(QDropEvent *event) {
    QListWidget::dropEvent(event);

    if (event->isAccepted()) {
        emit itemsReordered();
    }
}
