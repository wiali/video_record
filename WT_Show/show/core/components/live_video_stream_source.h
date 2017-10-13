#pragma once
#ifndef VIDEO_WIDGET_SOURCE_H
#define VIDEO_WIDGET_SOURCE_H

#include <QObject>
#include <QOpenGLContext>
#include <QTimer>
#include "model/video_stream_source_model.h"

namespace capture {
namespace components {

class LiveVideoStreamSource : public QObject
{
    Q_OBJECT
public:
    explicit LiveVideoStreamSource(QSharedPointer<model::VideoStreamSourceModel> model, QObject *parent = 0);

    void setSourcePipeline(QSharedPointer<video::source::SourcePipeline> sourcePipeline);    

    QSharedPointer<QOpenGLFramebufferObject> frameBuffer() const;
    QSize frameSize() const;

public slots:
    void updateFrameBuffer();
    void restartFreezeDetectionTimer();
    void stopFreezeDetection();

signals:
    void updated(QString sourceName);
    void streamFrozen(capture::common::VideoSourceInfo videoSource);

private slots:
    void onFrameReady(int sourceIndex, const QSize &size);
    void onIlluminationCorrectionEnabledChanged(bool isEnabled);
    void onAutoWhiteBalanceEnabledChanged(bool isEnabled);
    void onColorCorrectionModeChanged(capture::model::VideoStreamSourceModel::ColorCorrectionMode mode);
    void onFreezeDetectionTimerTimeout();

private:
    QSharedPointer<model::VideoStreamSourceModel> m_model;
    QMap<QOpenGLContext*, QSharedPointer<QOpenGLFramebufferObject>> m_frameBuffers;
    QSize m_frameSize;
    bool m_frameReadyForUpdate;
    QMetaObject::Connection m_sourceConnection;
    QSharedPointer<video::source::SourcePipeline> m_sourcePipeline;
    int m_frameCount;
    int m_framesSinceLastFreezeCheck;
    QTimer m_freezeDetectionTimer;
};

} // namespace components
} // namespace capture

#endif // VIDEO_WIDGET_SOURCE_H
