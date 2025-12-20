#ifndef PANELFRAME_H
#define PANELFRAME_H

#include <QFrame>

class PanelFrame : public QFrame {
    Q_OBJECT

public:
    explicit PanelFrame(QWidget *parent = nullptr);
    ~PanelFrame() override = default;

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // PANELFRAME_H
