#ifndef MODULEDELEGATE_H
#define MODULEDELEGATE_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
#include <functional>
#include "def.h"
#include "event.h"

class Module;
class EventBus;

typedef std::function<bool (Event&)> CallBack;

class ModuleDelegate : public QObject
{
    Q_OBJECT
    friend class Module;
private:
    explicit ModuleDelegate(Module* pDelegator, EventBus* pEventBus = nullptr);

public:
    void subscribeToEvtByName( const QString& strEvtName, CallBack pfListener );
    void unsubscribeToEvtByName( const QString& strEvtName );

    void dispatchEvt(const Event &rcEvt  ) const;

    void detach();
    void attach(EventBus *pcEventBus);

public slots:
    bool fire( QSharedPointer<Event> pcEvt );

protected:
    bool xIsListenToEvt(const QString& strEvtName);

    ADD_CLASS_FIELD( QString, strModuleName, getModuleName, setModuleName )
    ADD_CLASS_FIELD_NOSETTER(EventBus*, pEvtBus, getEvtBus)
    ADD_CLASS_FIELD_PRIVATE( CONCATE(QMap<QString, CallBack>), cListeningEvts )    
    ADD_CLASS_FIELD_PRIVATE(Module*, pDelegator)
};

#endif // MODULEDELEGATE_H
