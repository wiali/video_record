#pragma once
#ifndef CHANGECAMERASTATEEVENT_H
#define CHANGECAMERASTATEEVENT_H

#include <QEvent>

#include "common/video_source_info.h"

namespace capture {
namespace event {

class StartVideoStreamingEvent : public QEvent
{
public:
    StartVideoStreamingEvent(QVector<common::VideoSourceInfo> videoSources);

    static QEvent::Type type();
    void dispatch();

    inline QVector<common::VideoSourceInfo> videoSources() const { return m_videoSources; }
private:
    QVector<common::VideoSourceInfo> m_videoSources;
};

} // namespace event
} // namespace capture

#endif // CHANGECAMERASTATEEVENT_H
