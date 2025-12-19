#ifndef MODLISTWIDGET_H
#define MODLISTWIDGET_H

#include "../utils/ModManager.h"
#include "DraggableModList.h"
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

class ModListWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModListWidget(QWidget *parent = nullptr);
    ~ModListWidget() override = default;

    void setModManager(ModManager *modManager);
    void refreshModList();
    void setLoadingState(bool loading, const QString &message = "Loading mods");

signals:
    void modSelectionChanged(int selectedRow, int totalMods);

public slots:
    void onAddModClicked();
    void onRemoveModClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

private slots:
    void onModsChanged();
    void onModEnabledChanged(const QString &modId, bool enabled);
    void onItemsReordered();
    void updateLoadingAnimation();

private:
    void setupUi();
    QString getSelectedModId() const;
    int getSelectedRow() const;

    ModManager *m_modManager = nullptr;
    DraggableModList *m_modList;

    QLabel *m_loadingLabel;
    QTimer *m_loadingTimer;
    int m_loadingDots = 0;

    bool m_updating = false;
};

#endif // MODLISTWIDGET_H
