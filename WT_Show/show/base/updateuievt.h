#ifndef UPDATEUIEVT_H
#define UPDATEUIEVT_H

#include "event.h"

class UpdateUIEvt : public Event
{
    /// virtual copy pattern, please add this macro to all the subclass
    CLONABLE(UpdateUIEvt)

public:
    UpdateUIEvt();

    QString getCommandName();
    void setCommandName(const QString& strCommandName);
};

#endif // UPDATEUIEVT_H
