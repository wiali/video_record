#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <QSharedPointer>
#include "def.h"
#include "eventparam.h"

class Module;
class EventBus;

 /**********************************************************************************************************************
 @brief: The Event class represents an event.
 *  If you want to create an custom event by inherit Event, you MUST re implement the 'clone' method in this class.
 *  This can be done by adding VIRTUAL_COPY_PATTERN(subclass name) in the subclass. 
 *  Otherwise the application may crash.
 ***********************************************************************************************************************/
class Event
{
    /// virtual copy pattern, please add this macro to all the subclass
    CLONABLE(Event)

public:
    Event( const QString& strEvtName );
    Event();
    virtual ~Event() {}

    bool hasParameter(QString strParam) const;
    QVariant getParameter(const QString& strParam ) const;
    bool setParameter(const QString& strParam, const QVariant& rvValue);
    void dispatch(EventBus *pcEventBus = nullptr) const;

protected:
    ADD_CLASS_FIELD(QString, strEvtName, getEvtName, setEvtName) 
    ADD_CLASS_FIELD_NOSETTER(EventParam, cParameters, getParameters)
};

Q_DECLARE_METATYPE(QSharedPointer<Event>)

#endif // EVENT_H
