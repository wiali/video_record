#pragma once
#ifndef CAPTUREEVENT_H
#define CAPTUREEVENT_H

#include <QEvent>
#include <QRectF>

#include <ink_data.h>

#include "common/video_source_info.h"
#include "model/video_stream_source_model.h"

namespace capture {
namespace event {

class CaptureFrameEvent : public QEvent
{
public:
    CaptureFrameEvent(QSharedPointer<InkData> inkData, QRectF viewport, bool captureWithFlash,
                      bool captureNextFrame, QVector<common::VideoSourceInfo> videoSources,
                      model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode);

    static QEvent::Type type();
    void dispatch();

    inline bool captureWithFlash() const { return m_captureWithFlash; }
    inline bool captureNextFrame() const { return m_captureNextFrame; }
    inline QVector<common::VideoSourceInfo> videoSources() const { return m_videoSources; }
    inline QRectF viewport() const { return m_viewport; }
    inline QSharedPointer<InkData> inkData() const { return m_inkData; }
    inline model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode() const { return m_colorCorrectionMode; }

private:
    bool m_captureWithFlash;
    bool m_captureNextFrame;
    QVector<common::VideoSourceInfo> m_videoSources;
    QRectF m_viewport;
    QSharedPointer<InkData> m_inkData;
    model::VideoStreamSourceModel::ColorCorrectionMode m_colorCorrectionMode;
};

} // namespace event
} // namespace capture

#endif // CAPTUREEVENT_H
