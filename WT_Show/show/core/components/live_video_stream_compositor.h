#pragma once
#ifndef LIVEVIDEOSTREAMCOMPOSITOR_H
#define LIVEVIDEOSTREAMCOMPOSITOR_H

#include <QObject>
#include <QOffscreenSurface>
#include <QQueue>
#include <QMutex>

#include <video_source/videopipeline.h>

#include "live_video_stream_source.h"
#include "model/live_capture_model.h"

namespace capture {
namespace components {

class LiveVideoStreamCompositor : public QObject
{
    Q_OBJECT
public:
    explicit LiveVideoStreamCompositor(QSharedPointer<model::LiveCaptureModel> model, QObject *parent = nullptr);

    struct VideoStreamMapping
    {
        QRect source;
        QRect destination;
    };

    typedef QHash<common::VideoSourceInfo, VideoStreamMapping> VideoStreamMappings;

    VideoStreamMappings videoStreamMappings(const QRectF &widgetRectangle, const QRectF &primarySourceRectangle, const QRectF &primaryTransformRectangle,
                                            const QTransform &primaryTransform, bool invertDestinationRect, bool useWidgetRectangleForSecondaryStreams);
    VideoStreamMappings videoStreamMappings(const QRect& widgetRectangle, const QTransform &primaryTransform);

    void blitFramebuffer(QOpenGLFramebufferObject *target, VideoStreamMappings mappings);

    QSharedPointer<video::source::VideoPipeline> videoPipeline() const;

    QSize frameSize() const;

public slots:

    void requestCapture(const QSize &size = QSize(), const QRectF& viewport = QRectF(0, 0, 1, 1), bool waitForNextFrame = false, bool keepAspectRatio = false);
    void restartFreezeDetectionTimers();
    void stopFreezeDetection();
    void attachSourcePipelines();
    void detachSourcePipelines();

signals:

    void updated();
    void captureReady(QImage image);
    void streamFrozen(capture::common::VideoSourceInfo videoSource);

private slots:
    void onVideoStreamStateChanged(capture::model::LiveCaptureModel::VideoStreamState state);
    void onVideoStreamSourcesChanged(QVector<QSharedPointer<capture::model::VideoStreamSourceModel>> videoStreamSources);
    void onSourceUpdated(QString sourceName);
private:
    void parseSlotPositions();

    struct CaptureItem;

    QHash<common::VideoSourceInfo, QSharedPointer<LiveVideoStreamSource>> m_sources;
    QHash<common::VideoSourceInfo, QVector<QMetaObject::Connection>> m_sourceConnections;

    QSharedPointer<model::LiveCaptureModel> m_model;

    QVector<QRectF> m_slotPositions;

    QSharedPointer<video::source::VideoPipeline> m_videoPipeline;
    QQueue<CaptureItem> m_captureImageQueue;
    QMutex m_captureMutex;
    QImage m_capturedImage;
    QStringList m_missingFirstFrameList;

    bool m_suspendWhenInking;
};

} // namespace components
} // namespace capture

#endif // LIVEVIDEOSTREAMCOMPOSITOR_H
