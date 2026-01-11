#ifndef CONFLICTDETAILMODALCONTENT_H
#define CONFLICTDETAILMODALCONTENT_H

#include "../BaseModalContent.h"
#include "../../utils/ModConflictDetector.h"
#include <QString>

class QListWidget;
class QLabel;
class QVBoxLayout;

class ConflictDetailModalContent : public BaseModalContent {
    Q_OBJECT

public:
    explicit ConflictDetailModalContent(const QString &modName,
                                       const ConflictInfo &conflictInfo,
                                       QWidget *parent = nullptr);

private:
    void setupUi();
    QString formatModListText() const;

    QString m_modName;
    ConflictInfo m_conflictInfo;
    QListWidget *m_fileList;
};

#endif // CONFLICTDETAILMODALCONTENT_H
