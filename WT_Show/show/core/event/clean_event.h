#pragma once
#ifndef CLEAN_EVENT_H
#define CLEAN_EVENT_H

#include <QEvent>
#include <QSharedPointer>

namespace capture {
namespace event {

class CleanEvent : public QEvent
{
public:
    explicit CleanEvent();

    static QEvent::Type type();
    void dispatch();
};

} // namespace event
} // namespace capture

#endif // CLEAN_EVENT_H
