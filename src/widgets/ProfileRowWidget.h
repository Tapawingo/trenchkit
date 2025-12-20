#ifndef PROFILEROWWIDGET_H
#define PROFILEROWWIDGET_H

#include <QWidget>
#include <QString>

class QLabel;

class ProfileRowWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProfileRowWidget(const QString &profileId, const QString &profileName, QWidget *parent = nullptr);
    ~ProfileRowWidget() override = default;

    void setActive(bool active);
    void setSelected(bool selected);
    QString profileId() const { return m_profileId; }

signals:
    void exportRequested(const QString &profileId);
    void renameRequested(const QString &profileId);
    void deleteRequested(const QString &profileId);
    void clicked(const QString &profileId);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void setupUi(const QString &profileName);
    void updateStyling();

    QString m_profileId;
    bool m_isActive = false;
    bool m_isSelected = false;

    QLabel *m_iconLabel;
    QLabel *m_nameLabel;
};

#endif // PROFILEROWWIDGET_H
