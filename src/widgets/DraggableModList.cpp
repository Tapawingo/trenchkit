#include "DraggableModList.h"
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

DraggableModList::DraggableModList(QWidget *parent)
    : QListWidget(parent)
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void DraggableModList::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        bool hasValidFile = false;
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile() && isValidModFile(url.toLocalFile())) {
                hasValidFile = true;
                break;
            }
        }

        if (hasValidFile) {
            event->acceptProposedAction();
            return;
        }
    }

    QListWidget::dragEnterEvent(event);
}

void DraggableModList::dragMoveEvent(QDragMoveEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }

    QListWidget::dragMoveEvent(event);
}

void DraggableModList::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        QStringList filePaths;
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                if (isValidModFile(filePath)) {
                    filePaths.append(filePath);
                }
            }
        }

        if (!filePaths.isEmpty()) {
            event->acceptProposedAction();
            emit filesDropped(filePaths);
            return;
        }
    }

    QListWidget::dropEvent(event);

    if (event->isAccepted()) {
        emit itemsReordered();
    }
}

bool DraggableModList::isValidModFile(const QString &filePath) const {
    QString lower = filePath.toLower();
    return lower.endsWith(".pak") ||
           lower.endsWith(".zip") ||
           lower.endsWith(".rar") ||
           lower.endsWith(".7z") ||
           lower.endsWith(".tar.gz") ||
           lower.endsWith(".tar.bz2") ||
           lower.endsWith(".tar.xz");
}
