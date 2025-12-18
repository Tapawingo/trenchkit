#ifndef MODLISTWIDGET_H
#define MODLISTWIDGET_H

#include "../utils/ModManager.h"
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

class ModListWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModListWidget(QWidget *parent = nullptr);
    ~ModListWidget() override = default;

    void setModManager(ModManager *modManager);
    void refreshModList();

signals:
    void modSelectionChanged(const QString &modId);

private slots:
    void onModsChanged();
    void onModEnabledChanged(const QString &modId, bool enabled);
    void onAddModClicked();
    void onRemoveModClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

private:
    void setupUi();
    QString getSelectedModId() const;
    int getSelectedRow() const;

    ModManager *m_modManager = nullptr;
    QListWidget *m_modList;
    QPushButton *m_addButton;
    QPushButton *m_removeButton;
    QPushButton *m_moveUpButton;
    QPushButton *m_moveDownButton;

    bool m_updating = false; // Prevent recursive updates
};

#endif // MODLISTWIDGET_H
