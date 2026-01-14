#ifndef CONFLICTTOOLTIP_H
#define CONFLICTTOOLTIP_H

#include "common/widgets/GradientFrame.h"

class QLabel;

class ConflictTooltip : public GradientFrame {
    Q_OBJECT

public:
    explicit ConflictTooltip(QWidget *parent = nullptr);
    void setText(const QString &text);
    void showAt(const QPoint &globalPos);

private:
    QLabel *m_label = nullptr;
};

#endif // CONFLICTTOOLTIP_H
