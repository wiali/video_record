#include "capture_frame_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

CaptureFrameEvent::CaptureFrameEvent(QSharedPointer<InkData> inkData, QRectF viewport, bool captureWithFlash,
                                     bool captureNextFrame, QVector<common::VideoSourceInfo> videoSources,
                                     model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode)
    : QEvent(type())
    , m_videoSources(videoSources)
    , m_captureWithFlash(captureWithFlash)
    , m_captureNextFrame(captureNextFrame)
    , m_viewport(viewport)
    , m_inkData(inkData)
    , m_colorCorrectionMode(colorCorrectionMode)
{ }

QEvent::Type CaptureFrameEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void CaptureFrameEvent::dispatch() {
    qDebug() << "Dispatching event" << typeid(*this).name()
             << "viewport" << m_viewport
             << "captureWithFlash" << m_captureWithFlash
             << "captureWithFlash" << m_captureNextFrame
             << "colorCorrectionMode" << m_colorCorrectionMode
             << "videoSources" << m_videoSources;
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
