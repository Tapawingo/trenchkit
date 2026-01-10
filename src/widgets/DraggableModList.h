#ifndef DRAGGABLEMODLIST_H
#define DRAGGABLEMODLIST_H

#include <QListWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>

class DraggableModList : public QListWidget {
    Q_OBJECT

public:
    explicit DraggableModList(QWidget *parent = nullptr);
    ~DraggableModList() override = default;

signals:
    void itemsReordered();
    void filesDropped(const QStringList &filePaths);

protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

private:
    bool isValidModFile(const QString &filePath) const;
};

#endif // DRAGGABLEMODLIST_H
