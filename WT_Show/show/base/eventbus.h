#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <QList>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
#include "def.h"
#include "event.h"
#include "module.h"

class ModuleDelegate;
class EventBus : public QObject
{
    Q_OBJECT
private:
    EventBus();

public:
    static EventBus *create();
    bool registerModule(ModuleDelegate *pcModule);
    bool unregisterModule(ModuleDelegate *pcModule);

public slots:
    void post(const Event &rcEvt) const;

signals:
    void eventTriggered( QSharedPointer<Event> pcEvt ) const;

    SINGLETON_PATTERN_DECLARE(EventBus)
};

#endif // EVTBUS_H
