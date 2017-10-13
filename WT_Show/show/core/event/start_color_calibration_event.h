#pragma once
#ifndef STARTCOLORCALIBRATIONEVENT_H
#define STARTCOLORCALIBRATIONEVENT_H

#include <QEvent>

namespace capture {
namespace event {

class StartColorCalibrationEvent : public QEvent
{
public:
    StartColorCalibrationEvent();

    static QEvent::Type type();
    void dispatch();
};

} // namespace event
} // namespace capture

#endif // STARTCOLORCALIBRATIONEVENT_H
