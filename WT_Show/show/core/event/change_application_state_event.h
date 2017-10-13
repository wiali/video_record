#pragma once
#ifndef SUSPENDAPPLICATIONEVENT_H
#define SUSPENDAPPLICATIONEVENT_H

#include <QEvent>

namespace capture {
namespace event {

class ChangeApplicationStateEvent : public QEvent
{
public:
    ChangeApplicationStateEvent(bool suspended);

    static QEvent::Type type();
    void dispatch();

    inline bool suspended() const { return m_suspended; }
private:

    bool m_suspended;
};

} // namespace event
} // namespace capture

#endif // SUSPENDAPPLICATIONEVENT_H
