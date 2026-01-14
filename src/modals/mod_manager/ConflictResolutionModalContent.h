#ifndef CONFLICTRESOLUTIONMODALCONTENT_H
#define CONFLICTRESOLUTIONMODALCONTENT_H

#include "common/modals/BaseModalContent.h"
#include "core/models/ModInfo.h"

class QLabel;
class QPushButton;

class ConflictResolutionModalContent : public BaseModalContent {
    Q_OBJECT

public:
    enum class Action {
        Ignore,
        Overwrite,
        Duplicate
    };

    explicit ConflictResolutionModalContent(const ModInfo &incoming,
                                           const ModInfo &existing,
                                           bool checksumMatch,
                                           QWidget *parent = nullptr);

    Action selectedAction() const { return m_action; }

private:
    void setupUi();
    QString buildMessage() const;

    ModInfo m_incoming;
    ModInfo m_existing;
    bool m_checksumMatch = false;
    Action m_action = Action::Ignore;
};

#endif // CONFLICTRESOLUTIONMODALCONTENT_H
