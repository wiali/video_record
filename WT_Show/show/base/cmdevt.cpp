#include "cmdevt.h"

CmdEvt::CmdEvt(const QString& strCommandName) :
    Event(_EXE_COMMAND_REQUEST_EVENT)
{
    Event::setParameter("command_name", strCommandName);
}

QString CmdEvt::getCommandName()
{
    return Event::getParameter("command_name").toString();
}

void CmdEvt::setCommandName(const QString& strCommandName)
{
    Event::setParameter("command_name", strCommandName);
}
