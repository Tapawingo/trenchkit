#ifndef MODROWWIDGET_H
#define MODROWWIDGET_H

#include "../utils/ModInfo.h"
#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

class ConflictTooltip;
struct ConflictInfo;

class ModRowWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModRowWidget(const ModInfo &mod, QWidget *parent = nullptr);
    ~ModRowWidget() override = default;

    QString modId() const { return m_modId; }
    void updateModInfo(const ModInfo &mod);
    void setSelected(bool selected);
    void setUpdateAvailable(bool available, const QString &version);
    void hideUpdateButton();
    void setConflictInfo(const ConflictInfo &info);
    void clearConflictIndicator();

signals:
    void enabledChanged(const QString &modId, bool enabled);
    void renameRequested(const QString &modId);
    void editMetaRequested(const QString &modId);
    void removeRequested(const QString &modId);
    void updateRequested(const QString &modId);
    void conflictDetailsRequested(const QString &modId);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUi(const ModInfo &mod);
    void updateStyling();
    void updateNameEliding();
    QString formatConflictTooltip(const ConflictInfo &info) const;

    QString m_modId;
    QString m_fullModName;
    QCheckBox *m_enabledCheckBox;
    QLabel *m_nameLabel;
    QLabel *m_dateLabel;
    QPushButton *m_updateButton;
    QPushButton *m_conflictButton;
    ConflictTooltip *m_conflictTooltip = nullptr;
    QString m_conflictTooltipText;
    bool m_selected = false;
};

#endif // MODROWWIDGET_H
