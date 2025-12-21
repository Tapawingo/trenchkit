#ifndef ACTIONSWIDGET_H
#define ACTIONSWIDGET_H

#include <QWidget>
#include <QString>

class QPushButton;
class QFrame;

class ActionsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ActionsWidget(QWidget *parent = nullptr);
    ~ActionsWidget() override = default;

    void setFoxholeInstallPath(const QString &path);

signals:
    void addModRequested();
    void removeModRequested();
    void moveUpRequested();
    void moveDownRequested();
    void errorOccurred(const QString &error);

public slots:
    void onModSelectionChanged(int selectedRow, int totalMods);

private slots:
    void onAddModClicked();
    void onRemoveModClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onExploreFolderClicked();

private:
    void setupUi();
    void setupConnections();
    QFrame* createSeparator();

    QString m_foxholeInstallPath;
    int m_selectedRow = -1;
    int m_totalMods = 0;

    QPushButton *m_addButton;
    QPushButton *m_removeButton;
    QPushButton *m_moveUpButton;
    QPushButton *m_moveDownButton;
    QPushButton *m_exploreFolderButton;
};

#endif // ACTIONSWIDGET_H
