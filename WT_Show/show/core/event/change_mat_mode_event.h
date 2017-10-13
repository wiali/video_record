#pragma once
#ifndef CHANGE_MAT_MODE_EVENT_H
#define CHANGE_MAT_MODE_EVENT_H

#include <QEvent>

namespace capture {
namespace event {

class ChangeMatModeEvent : public QEvent
{
    Q_GADGET

public:
    enum MatMode
    {
        None,
        LampOff,
        LampOn,
        Desktop,
        Flash,
        Reprojection
    };

    Q_ENUM(MatMode)

    ChangeMatModeEvent(event::ChangeMatModeEvent::MatMode mode);

    inline ChangeMatModeEvent::MatMode mode() const { return m_mode; }
    static QEvent::Type type();

    void dispatch();

private:

    ChangeMatModeEvent::MatMode m_mode;
};

} // namespace event
} // namespace capture

Q_DECLARE_METATYPE(capture::event::ChangeMatModeEvent::MatMode)

#endif // CHANGE_MAT_MODE_EVENT_H
