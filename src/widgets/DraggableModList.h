#ifndef DRAGGABLEMODLIST_H
#define DRAGGABLEMODLIST_H

#include <QListWidget>

class DraggableModList : public QListWidget {
    Q_OBJECT

public:
    explicit DraggableModList(QWidget *parent = nullptr);
    ~DraggableModList() override = default;

signals:
    void itemsReordered();

protected:
    void dropEvent(QDropEvent *event) override;
};

#endif // DRAGGABLEMODLIST_H
