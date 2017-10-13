#include "start_color_calibration_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

StartColorCalibrationEvent::StartColorCalibrationEvent()
    : QEvent(type())
{}

QEvent::Type StartColorCalibrationEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void StartColorCalibrationEvent::dispatch() {
    qDebug() << "Dispatching event" << typeid(*this).name();
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
