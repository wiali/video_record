#include "module.h"
#include "eventbus.h"
#include <QDebug>

Module::Module(EventBus *pcEventBus) :
    m_cDelegate(this, pcEventBus)
{    
}

void Module::subscribeToEvtByName( const QString& strEvtName, const CallBack& pfListener )
{
    return m_cDelegate.subscribeToEvtByName(strEvtName, pfListener);
}

void Module::unsubscribeToEvtByName( const QString& strEvtName )
{
    return m_cDelegate.unsubscribeToEvtByName(strEvtName);
}

void Module::dispatchEvt( Event& rcEvt )
{
    m_cDelegate.dispatchEvt(rcEvt);
}

void Module::setModuleName( QString strModuleName )
{
    m_cDelegate.setModuleName(strModuleName);
}

EventBus *Module::getEventBus()
{
    return m_cDelegate.getEvtBus();
}

void Module::detach()
{
    m_cDelegate.detach();
}

void Module::attach(EventBus *pcEventBus)
{
    m_cDelegate.attach(pcEventBus);
}
