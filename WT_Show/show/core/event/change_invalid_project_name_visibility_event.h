#pragma once
#ifndef SHOWINVALIDPROJECTNAMEEVENT_H
#define SHOWINVALIDPROJECTNAMEEVENT_H

#include <QEvent>
#include <QPoint>

namespace capture {
namespace event {

class ChangeInvalidProjectNameVisibilityEvent : public QEvent
{
public:
    ChangeInvalidProjectNameVisibilityEvent(bool visible, QPoint globalPosition);

    static QEvent::Type type();
    void dispatch();

    inline QPoint globalPosition() const { return m_globalPosition; }
    inline bool visible() const { return m_visible; }

private:

    QPoint m_globalPosition;
    bool m_visible;
};

} // namespace event
} // namespace capture

#endif // SHOWINVALIDPROJECTNAMEEVENT_H
