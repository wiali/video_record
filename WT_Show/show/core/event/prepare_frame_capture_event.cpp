#include "prepare_frame_capture_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

PrepareFrameCaptureEvent::PrepareFrameCaptureEvent(bool captureWithFlash, QVector<common::VideoSourceInfo> videoSources)
    : QEvent(type())
    , m_captureWithFlash(captureWithFlash)
    , m_videoSources(videoSources)
{ }

QEvent::Type PrepareFrameCaptureEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void PrepareFrameCaptureEvent::dispatch() {
    qDebug() << "Dispatching event" << typeid(*this).name()
             << "captureWithFlash" << m_captureWithFlash
             << "videoSources" << m_videoSources;

    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
