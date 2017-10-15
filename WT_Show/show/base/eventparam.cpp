#include "eventparam.h"
#include <QDebug>

EventParam::EventParam()
{
}

bool EventParam::hasParameter(QString strParam) const
{
    return m_cParameters.contains(strParam);
}

QVariant EventParam::getParameter(const QString& strParam ) const
{
    QVariant rvValue;
    if( m_cParameters.contains(strParam) )
    {
        rvValue = m_cParameters[strParam];
    }
    else
    {
        qWarning() << QString("Parameter %1 NOT found.").arg(strParam);
    }
    return rvValue;
}

bool EventParam::setParameter(const QString& strParam, const QVariant& rvValue)
{
    m_cParameters[strParam] = rvValue;
    return true;
}
