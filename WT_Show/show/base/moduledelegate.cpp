#include "moduledelegate.h"
#include "eventbus.h"
#include <QDebug>
#include <iostream>

using namespace std;

ModuleDelegate::ModuleDelegate(Module* pDelegator, EventBus* pEventBus)
{
    m_pDelegator = pDelegator;

    if(pEventBus == nullptr)
        m_pEvtBus = EventBus::getInstance();
    else
        m_pEvtBus = pEventBus;

    m_pEvtBus->registerModule(this);
    m_strModuleName = "undefined_module_name";
}

void ModuleDelegate::subscribeToEvtByName(const QString& strEvtName, CallBack pfListener )
{
    m_cListeningEvts.insert(strEvtName, pfListener);
    return;
}

void ModuleDelegate::unsubscribeToEvtByName( const QString& strEvtName )
{
    m_cListeningEvts.remove(strEvtName);
}

bool ModuleDelegate::fire( QSharedPointer<Event> pcEvt )
{
    QMap<QString, std::function<bool (Event&)>>::iterator p =
            m_cListeningEvts.find(pcEvt->getEvtName());

    if( p != m_cListeningEvts.end() )
        (p.value())(*pcEvt.data());

    return true;
}

bool ModuleDelegate::xIsListenToEvt( const QString& strEvtName )
{
    return m_cListeningEvts.contains(strEvtName);
}

void ModuleDelegate::dispatchEvt( const Event& rcEvt ) const
{
    if(m_pEvtBus != nullptr)
        m_pEvtBus->post(rcEvt);
}

void ModuleDelegate::detach()
{
    if(m_pEvtBus != nullptr)
        m_pEvtBus->unregisterModule(this);
    m_pEvtBus = nullptr;
}

void ModuleDelegate::attach(EventBus* pEventBus)
{
    if(pEventBus == nullptr)
        return;
    detach();
    m_pEvtBus = pEventBus;
    m_pEvtBus->registerModule(this);
}
