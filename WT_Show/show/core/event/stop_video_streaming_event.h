#pragma once
#ifndef STOP_VIDEO_STREAMING_EVENT_H
#define STOP_VIDEO_STREAMING_EVENT_H

#include <QEvent>

namespace capture {
namespace event {

class StopVideoStreamingEvent : public QEvent
{
public:
    explicit StopVideoStreamingEvent();

    static QEvent::Type type();
    void dispatch();
};

} // namespace event
} // namespace capture

#endif // STOP_VIDEO_STREAMING_EVENT_H
