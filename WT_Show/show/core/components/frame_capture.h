#pragma once
#ifndef FRAMECAPTURE_H
#define FRAMECAPTURE_H

#include <QObject>
#include <QPointer>
#include <QQueue>
#include <QRunnable>
#include <QThreadPool>
#include <QWaitCondition>
#include <QOpenGLFramebufferObject>

#include <atomic>
#include <image_enhancement/imageenhancement.h>
#include <depth_camera/depth_camera.h>

#include "components/live_video_stream_compositor.h"
#include "model/application_state_model.h"
#include "components/video_source_input.h"

namespace capture {
namespace components {

/*!
 * \brief The FrameCapture class performs the capture from all cameras (hires, depth and IR).
 * \details This class monitors occurences of PrepareFrameCaptureEvent to start preparation for the capture. Once the preparation is complete the
 * \details class waits for CaptureFrameEvent which should be sent once the countdown timer reaches zero and performs the capture.
 * \details Only single capture is being executed at the same time.
 */
class FrameCapture : public QObject, public QRunnable
{
    Q_OBJECT
public:
    /*!
     * \brief FrameCapture constructor.
     * \param videoPipeline Pointer to video pipeline.
     * \param model Application model object.
     * \param parent Parent object.
     */
    explicit FrameCapture(QSharedPointer<components::LiveVideoStreamCompositor> compositor,
                          QSharedPointer<model::ApplicationStateModel> model,
                          QObject *parent = 0);

    virtual ~FrameCapture();
    virtual void run() override;
    void abort();

signals:
    void captureFailed();

protected:

    virtual bool eventFilter(QObject *obj, QEvent *event);

private slots:

    void onMatModeStateChanged(capture::model::ApplicationStateModel::MatModeState matModeState);
    void onCaptureReady(QImage image);
    void updateDepthStreams();

private:
    struct CaptureData;
    struct CaptureResult;

    QVector<sensordata::SensorData> captureCameraData(QVector<common::VideoSourceInfo> videoSources, bool captureWithFlash);
    QVector<sensordata::SensorData> captureOrbbecData();
    QImage captureLiveFrame(bool captureNextFrame, const QRectF &viewport);
    void prepareCapture(bool captureWithFlash, QVector<common::VideoSourceInfo> videoSources);
    void performCapture(bool captureWithFlash, bool captureNextFrame, model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode,
                        QVector<common::VideoSourceInfo> videoSources, const QRectF &viewport, QSharedPointer<InkData> inkData);
    QSharedPointer<StageProject> createStageProject(QVector<common::VideoSourceInfo> videoSources, QRectF viewport, QSharedPointer<InkData> inkData, model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode,
                                                    CaptureResult captureResult);
    CaptureResult capture(bool captureWithFlash, bool captureNextFrame, const QRectF &viewport, QVector<common::VideoSourceInfo> videoSources);
    bool supportsOrbbecCapture(QVector<common::VideoSourceInfo> videoSources);
    void calcInkPosition(const QRectF& viewport, const QSize& captureImageSize, QSharedPointer<StageItem> stageItem, bool multiSources);

    QMutex m_orbbecMutex;
    QMutex m_mutex;
    QQueue<CaptureData> m_captureQueue;

    QSharedPointer<components::LiveVideoStreamCompositor> m_compositor;
    QSharedPointer<proapi::depth_camera::DepthCamera> m_depthCamera;
    QSharedPointer<image_enhancement::ImageEnhancement> m_imageEnhancement;
    QSharedPointer<model::ApplicationStateModel> m_model;
    QScopedPointer<QThreadPool> m_threadPool;
    QScopedPointer<QThread> m_depthCameraThread;
    QWaitCondition m_waitForFlashEvent;
    QMutex m_captureMutex;
    QMutex m_depthCameraMutex;

    QWaitCondition m_liveImageCaptureEvent;
    QImage m_liveCaptureImage;
    int m_preCaptureViewportDelay;
};

} // namespace components
} // namespace capture

#endif // FRAMECAPTURE_H
