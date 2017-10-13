#pragma once
#ifndef PREPAREFORCAPTUREEVENT_H
#define PREPAREFORCAPTUREEVENT_H

#include <QEvent>

#include "common/video_source_info.h"

namespace capture {
namespace event {

class PrepareFrameCaptureEvent : public QEvent
{
public:
    PrepareFrameCaptureEvent(bool captureWithFlash,
                             QVector<common::VideoSourceInfo> videoSources);

    static QEvent::Type type();
    void dispatch();

    inline bool captureWithFlash() const { return m_captureWithFlash; }
    inline QVector<common::VideoSourceInfo> videoSources() const { return m_videoSources; }

private:

    bool m_captureWithFlash;
    QVector<common::VideoSourceInfo> m_videoSources;
};

} // namespace event
} // namespace capture

#endif // PREPAREFORCAPTUREEVENT_H
