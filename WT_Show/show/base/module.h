#ifndef MODULE_H
#define MODULE_H

#include <QVector>
#include <QString>
#include <QSharedPointer>

#include "def.h"
#include "event.h"
#include "moduledelegate.h"

class EventBus;

class Module
{
public:
    Module(EventBus* pEventBus = nullptr);

    void subscribeToEvtByName(const QString& strEvtName, const CallBack& pListener);
    void unsubscribeToEvtByName( const QString& strEvtName );

    void dispatchEvt(Event &rcEvt );

    void setModuleName(QString strModuleName );

    EventBus* getEventBus();

    void detach();
    void attach(EventBus *pcEventBus);

    ADD_CLASS_FIELD_PRIVATE( ModuleDelegate, cDelegate )
};

#endif // MODULE_H
