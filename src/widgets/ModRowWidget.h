#ifndef MODROWWIDGET_H
#define MODROWWIDGET_H

#include "../utils/ModInfo.h"
#include <QWidget>
#include <QCheckBox>
#include <QLabel>

class ModRowWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModRowWidget(const ModInfo &mod, QWidget *parent = nullptr);
    ~ModRowWidget() override = default;

    QString modId() const { return m_modId; }
    void updateModInfo(const ModInfo &mod);
    void setSelected(bool selected);

signals:
    void enabledChanged(const QString &modId, bool enabled);

private:
    void setupUi(const ModInfo &mod);
    void updateStyling();

    QString m_modId;
    QCheckBox *m_enabledCheckBox;
    QLabel *m_nameLabel;
    QLabel *m_dateLabel;
    bool m_selected = false;
};

#endif // MODROWWIDGET_H
