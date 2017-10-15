#include "eventbus.h"
#include <QDebug>

SINGLETON_PATTERN_IMPLIMENT(EventBus)

EventBus::EventBus()
{
}

EventBus *EventBus::create()
{
    return new EventBus();
}

bool EventBus::registerModule(ModuleDelegate* pModule)
{
    connect(this, SIGNAL(eventTriggered(QSharedPointer<Event>) ),
            pModule, SLOT(fire(QSharedPointer<Event>) ),
            Qt::AutoConnection );
    return true;
}

bool EventBus::unregisterModule(ModuleDelegate *pcModule)
{
    return disconnect(this, nullptr, pcModule, nullptr);
}

void EventBus::post(const Event& rcEvt) const
{    
    QSharedPointer<Event> pcEvtCopy( rcEvt.clone() );

    /// notify modules
    emit eventTriggered(pcEvtCopy);
}

