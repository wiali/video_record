#include "stop_video_streaming_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

StopVideoStreamingEvent::StopVideoStreamingEvent()
    : QEvent(type())
{ }

QEvent::Type StopVideoStreamingEvent::type()
{
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void StopVideoStreamingEvent::dispatch()
{
    qDebug() << "Dispatching event" << typeid(*this).name();
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
