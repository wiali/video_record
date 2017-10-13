#pragma once
#ifndef LIVECAPTUREMODEL_H
#define LIVECAPTUREMODEL_H

#include <QObject>
#include <QHash>
#include <QRectF>
#include <QSharedPointer>

#include <ink_data.h>

#include "video_stream_source_model.h"
#include "common/video_source_info.h"

namespace capture {
namespace model {

class LiveCaptureModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVector<QSharedPointer<model::VideoStreamSourceModel>> videoStreamSources READ videoStreamSources CONSTANT)
    Q_PROPERTY(bool flashMode READ flashMode WRITE setFlashMode NOTIFY flashModeChanged)
    Q_PROPERTY(bool autoFix READ autoFix WRITE setAutoFix NOTIFY autoFixChanged)
    Q_PROPERTY(model::LiveCaptureModel::CaptureState captureState READ captureState WRITE setCaptureState NOTIFY captureStateChanged)
    Q_PROPERTY(QVector<common::VideoSourceInfo> selectedVideoStreamSources READ selectedVideoStreamSources
               WRITE setSelectedVideoStreamSources NOTIFY selectedVideoStreamSourcesChanged)
    Q_PROPERTY(model::LiveCaptureModel::PreCaptureMode preCaptureMode READ preCaptureMode WRITE setPreCaptureMode NOTIFY preCaptureModeChanged)
    Q_PROPERTY(QSharedPointer<InkData> inkData READ inkData CONSTANT)
    Q_PROPERTY(model::LiveCaptureModel::VideoStreamState videoStreamState READ videoStreamState WRITE setVideoStreamState NOTIFY videoStreamStateChanged)
    Q_PROPERTY(bool inking READ inking WRITE setInking NOTIFY inkingChanged)
    Q_PROPERTY(QRectF viewport READ viewport WRITE setViewport NOTIFY viewportChanged)
    Q_PROPERTY(int depthCameraIndex READ depthCameraIndex WRITE setDepthCameraIndex NOTIFY depthCameraIndexChanged)
    Q_PROPERTY(model::LiveCaptureModel::KeystoneCorrectionMode keystoneCorrectionMode READ keystoneCorrectionMode WRITE setKeystoneCorrectionMode NOTIFY keystoneCorrectionModeChanged)

public:
    explicit LiveCaptureModel(QObject *parent = 0);

    enum CaptureState
    {
        NotCapturing,
        PreparingCapture,
        CapturePrepared,
        Capturing,
        FinalizingCapture
    };

    Q_ENUM(CaptureState)

    enum PreCaptureMode
    {
        LampOff,
        LampOn,
        Desktop
    };

    Q_ENUM(PreCaptureMode)

    enum class VideoStreamState
    {
        Stopped,
        Starting,
        Running,
        FailedToStart,
        CalibrationDataMissing
    };

    Q_ENUM(VideoStreamState)    

    enum class KeystoneCorrectionMode
    {
        NoKeystoneCorrection,
        NonCroppedKeystoneCorrection,
        CroppedKeystoneCorrection
    };

    Q_ENUM(KeystoneCorrectionMode)

    inline QVector<QSharedPointer<model::VideoStreamSourceModel>> videoStreamSources () const { return m_videoStreamSources; }
    inline bool flashMode() const { return m_flashMode; }
    inline bool autoFix() const { return m_autoFix; }
    inline model::LiveCaptureModel::CaptureState captureState() const { return m_captureState; }
    inline model::LiveCaptureModel::PreCaptureMode preCaptureMode() const { return m_preCaptureMode; }
    inline QVector<common::VideoSourceInfo> selectedVideoStreamSources() const { return m_selectedVideoStreamSources; }
    inline QSharedPointer<InkData> inkData() const { return m_inkData; }
    inline QRectF viewport() const { return m_viewport; }
    inline model::LiveCaptureModel::VideoStreamState videoStreamState() const { return m_videoStreamState; }
    inline bool inking() const { return m_inking; }
    inline int depthCameraIndex() const { return m_depthCameraIndex; }
    inline model::LiveCaptureModel::KeystoneCorrectionMode keystoneCorrectionMode() const { return m_keystoneCorrectionMode; }

    bool supportsFlashCapture() const;
    bool supportsDepthCapture() const { return supportsFlashCapture(); }
    QSharedPointer<model::VideoStreamSourceModel> fullscreenVideoStreamModel() const;
    QSharedPointer<model::VideoStreamSourceModel> videoStreamSource(capture::common::VideoSourceInfo type) const;

signals:
    void flashModeChanged(bool flashMode);
    void autoFixChanged(bool autoFix);
    void captureStateChanged(capture::model::LiveCaptureModel::CaptureState captureState);
    void selectedVideoStreamSourcesChanged(QVector<capture::common::VideoSourceInfo> selectedVideoStreamSources);
    void preCaptureModeChanged(capture::model::LiveCaptureModel::PreCaptureMode preCaptureMode);
    void videoStreamStateChanged(capture::model::LiveCaptureModel::VideoStreamState state);
    void videoStreamSourcesChanged(QVector<QSharedPointer<capture::model::VideoStreamSourceModel>> videoStreamSources);
    void inkingChanged(bool inking);
    void viewportChanged(QRectF viewport);
    void depthCameraIndexChanged(int depthCameraIndex);
    void keystoneCorrectionModeChanged(capture::model::LiveCaptureModel::KeystoneCorrectionMode mode);

public slots:    
    void setFlashMode(bool flashMode);
    void setAutoFix(bool autoFix);
    void setCaptureState(capture::model::LiveCaptureModel::CaptureState captureState);
    void setSelectedVideoStreamSources(QVector<capture::common::VideoSourceInfo> selectedVideoStreamSources);
    void setPreCaptureMode(capture::model::LiveCaptureModel::PreCaptureMode preCaptureMode);
    void setVideoStreamState(capture::model::LiveCaptureModel::VideoStreamState state);
    void setVideoStreamSources(QVector<QSharedPointer<capture::model::VideoStreamSourceModel>> videoStreamSources);
    void setInking(bool inking);
    void setViewport(QRectF viewport);
    void setDepthCameraIndex(int depthCameraIndex);
    void setKeystoneCorrectionMode(capture::model::LiveCaptureModel::KeystoneCorrectionMode mode);

private:
    QVector<QSharedPointer<model::VideoStreamSourceModel>> m_videoStreamSources;
    bool m_flashMode;
    bool m_autoFix;
    model::LiveCaptureModel::CaptureState m_captureState;
    QVector<common::VideoSourceInfo> m_selectedVideoStreamSources;
    model::LiveCaptureModel::PreCaptureMode m_preCaptureMode;
    QSharedPointer<InkData> m_inkData;
    model::LiveCaptureModel::VideoStreamState m_videoStreamState;
    bool m_inking;
    QRectF m_viewport;
    int m_depthCameraIndex;    
    KeystoneCorrectionMode m_keystoneCorrectionMode;
    QTransform m_primaryStreamTransform;
};

} // namespace model
} // namespace capture

Q_DECLARE_METATYPE(capture::model::LiveCaptureModel::VideoStreamState)
Q_DECLARE_METATYPE(capture::model::LiveCaptureModel::CaptureState)
Q_DECLARE_METATYPE(capture::model::LiveCaptureModel::PreCaptureMode)

#endif // LIVECAPTUREMODEL_H
