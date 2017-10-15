#include "event.h"
#include "module.h"
#include <QDebug>
#include "eventbus.h"
#include <QSharedPointer>

Event::Event( const QString& strEvtName )
{
    this->m_strEvtName = strEvtName;
    static struct Initialize
    {
        Initialize() 
        {
            qRegisterMetaType<QSharedPointer<Event>>("QSharedPointer<Event>");
        }
    } initialize;
}

Event::Event()
{
    this->m_strEvtName = "UNKNOWN";
}

bool Event::hasParameter(QString strParam) const
{
    return m_cParameters.hasParameter(strParam);
}

QVariant Event::getParameter(const QString& strParam ) const
{
    return m_cParameters.getParameter(strParam);
}

bool Event::setParameter(const QString& strParam, const QVariant& rvValue)
{
    m_cParameters.setParameter(strParam, rvValue);
    return true;
}

void Event::dispatch(EventBus* pEventBus) const
{
    if(pEventBus == nullptr)
        EventBus::getInstance()->post(*this);
    else
        pEventBus->post(*this);
}
