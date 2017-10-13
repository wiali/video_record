#include "start_video_streaming_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

StartVideoStreamingEvent::StartVideoStreamingEvent(QVector<common::VideoSourceInfo> videoSources)
    : QEvent(type())
    , m_videoSources(videoSources)
{ }

QEvent::Type StartVideoStreamingEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void StartVideoStreamingEvent::dispatch() {
    qDebug() << "Dispatching event" << typeid(*this).name() << ":" << m_videoSources;
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture

