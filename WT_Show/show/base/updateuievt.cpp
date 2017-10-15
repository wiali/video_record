#include "updateuievt.h"

UpdateUIEvt::UpdateUIEvt() : Event(_UPDATE_UI_REQUEST_EVENT)
{
}

QString UpdateUIEvt::getCommandName()
{
    return getParameter("command_name").toString();
}

void UpdateUIEvt::setCommandName(const QString& strCommandName)
{
    setParameter("command_name", strCommandName);
}

