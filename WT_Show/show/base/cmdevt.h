#ifndef CMDEVT_H
#define CMDEVT_H

#include <QString>
#include "event.h"

class CmdEvt : public Event
{
    /// virtual copy pattern, please add this macro to all the subclass
    CLONABLE(CmdEvt)

public:
    CmdEvt(const QString& strCommandName);

    QString getCommandName();
    void setCommandName(const QString& strCommandName);
};

#endif // CMDEVT_H
