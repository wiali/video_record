#ifndef EVENTPARAM_H
#define EVENTPARAM_H

#include <QString>
#include <QMap>
#include <QVariant>
#include "def.h"

class EventParam
{
public:
    EventParam();

    bool hasParameter(QString strParam) const;
    QVariant getParameter(const QString& strParam ) const;
    bool setParameter(const QString& strParam, const QVariant& rvValue);

    ADD_CLASS_FIELD_PRIVATE( CONCATE(QMap<QString,QVariant>), cParameters)
};

#endif // EVENTPARAM_H
