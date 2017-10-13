#include "live_capture_model.h"

#include <QDebug>

#include <global_utilities.h>

#include "common/utilities.h"

namespace capture {
namespace model {

LiveCaptureModel::LiveCaptureModel(QObject *parent)
    : QObject(parent)        
    , m_flashMode(false)
    , m_autoFix(true)
    , m_captureState(CaptureState::NotCapturing)    
    , m_preCaptureMode(model::LiveCaptureModel::LampOff)
    , m_inkData(new InkData())
    , m_videoStreamState(VideoStreamState::Stopped)
    , m_inking(false)
    , m_viewport(0, 0, 1, 1)
    , m_depthCameraIndex(-1)
    , m_keystoneCorrectionMode(KeystoneCorrectionMode::NoKeystoneCorrection) {
    // Needed for proper cross-thread signals
    static struct Initialize {
        Initialize() {
            qRegisterMetaType<capture::model::LiveCaptureModel::CaptureState>();
            qRegisterMetaType<capture::model::LiveCaptureModel::PreCaptureMode>();
            qRegisterMetaType<capture::model::LiveCaptureModel::VideoStreamState>();
            qRegisterMetaType<capture::model::LiveCaptureModel::KeystoneCorrectionMode>();
            qRegisterMetaType<QVector<capture::common::VideoSourceInfo>>();
            qRegisterMetaType<QSharedPointer<InkData>>();
            qRegisterMetaType<QSharedPointer<capture::model::VideoStreamSourceModel>>();
            qRegisterMetaType<QVector<QSharedPointer<capture::model::VideoStreamSourceModel>>>();
        }
    } initialize;    

    // SPROUT-18613 -  The default values of auto-fix and flash should be inverted
    auto settings = GlobalUtilities::applicationSettings("capture");
    m_flashMode = settings->value("flash_mode_initial_value", false).toBool();
    m_autoFix = settings->value("auto_fix_initial_value", false).toBool();    
}

void model::LiveCaptureModel::setSelectedVideoStreamSources(QVector<common::VideoSourceInfo> selectedVideoStreamSources) {
    // These types always takes precendece in order as they are defined
    static QVector<common::VideoSourceInfo> priorityTypes ({ common::VideoSourceInfo::DownwardFacingCamera(),
                                                             common::VideoSourceInfo::SproutCamera(),
                                                             common::VideoSourceInfo::ForwardFacingCamera()});

    QVector<common::VideoSourceInfo> sortedList;

    for (auto priorityType : priorityTypes) {
        for (auto source : selectedVideoStreamSources) {
            if (source == priorityType) {
                sortedList << source;
                selectedVideoStreamSources.removeOne(source);
                break;
            }
        }
    }

    for (auto source : selectedVideoStreamSources) {
        sortedList << source;
    }

    if (m_selectedVideoStreamSources != sortedList) {
        m_selectedVideoStreamSources = sortedList;

        qInfo() << this << "Changing selected video stream sources to" << m_selectedVideoStreamSources;

        emit selectedVideoStreamSourcesChanged(m_selectedVideoStreamSources);
    }
}

void model::LiveCaptureModel::setFlashMode(bool flashMode) {
    if (m_flashMode != flashMode)     {
        m_flashMode = flashMode;
        qInfo() << this << "Changing flash mode" << m_flashMode;

        emit flashModeChanged(m_flashMode);
    }
}

void model::LiveCaptureModel::setAutoFix(bool autoFix) {
    if (m_autoFix != autoFix) {
        m_autoFix = autoFix;
        qInfo() << this << "Changing auto fix" << m_autoFix;

        emit autoFixChanged(m_autoFix);
    }
}

void model::LiveCaptureModel::setCaptureState(CaptureState captureState) {
    if (m_captureState != captureState) {
        m_captureState = captureState;
        qInfo() << this << "Changing capture state to" << m_captureState;

        emit captureStateChanged(m_captureState);
    }
}

void model::LiveCaptureModel::setPreCaptureMode(model::LiveCaptureModel::PreCaptureMode preCaptureMode) {
    if (m_preCaptureMode != preCaptureMode) {
        m_preCaptureMode = preCaptureMode;
        qInfo() << this << "Changing pre-capture mode to" << m_preCaptureMode;

        emit preCaptureModeChanged(m_preCaptureMode);
    }
}

void model::LiveCaptureModel::setVideoStreamState(model::LiveCaptureModel::VideoStreamState videoStreamState) {
    if (m_videoStreamState != videoStreamState) {
        m_videoStreamState = videoStreamState;

        qInfo() << this << "Video stream state changed to " << m_videoStreamState;

        emit videoStreamStateChanged(m_videoStreamState);
    }
}

bool model::LiveCaptureModel::supportsFlashCapture() const {
    return selectedVideoStreamSources().contains(common::VideoSourceInfo::DownwardFacingCamera()) ||
           selectedVideoStreamSources().contains(common::VideoSourceInfo::SproutCamera());
}

QSharedPointer<model::VideoStreamSourceModel> model::LiveCaptureModel::videoStreamSource(common::VideoSourceInfo type) const {
    QSharedPointer<model::VideoStreamSourceModel> result;

    for (auto streamSource : videoStreamSources()) {
        result = streamSource->videoSource() == type ? streamSource : result;
    }

    return result;
}

QSharedPointer<model::VideoStreamSourceModel> model::LiveCaptureModel::fullscreenVideoStreamModel() const {
    QSharedPointer<model::VideoStreamSourceModel> result;

    for(auto videoStreamSource : videoStreamSources()) {
        // First position is always reserved for primary (full screen) source
        result = selectedVideoStreamSources().first() == videoStreamSource->videoSource() ? videoStreamSource : result;
    }

    return result;
}

void model::LiveCaptureModel::setVideoStreamSources(QVector<QSharedPointer<model::VideoStreamSourceModel>> videoStreamSources) {
    if (m_videoStreamSources != videoStreamSources) {
        m_videoStreamSources = videoStreamSources;

        QVector<common::VideoSourceInfo> streamSources;

        for (auto source : m_videoStreamSources) {
            streamSources << source->videoSource();
        }

        qInfo() << this << "Video stream sources changed to " << streamSources;

        emit videoStreamSourcesChanged(m_videoStreamSources);
    }
}

void model::LiveCaptureModel::setInking(bool inking) {
    if (m_inking != inking) {
        m_inking = inking;
        emit inkingChanged(m_inking);
    }
}

void model::LiveCaptureModel::setViewport(QRectF viewport) {
    if (m_viewport != viewport) {
        m_viewport = viewport;

        qDebug() << this << "Viewport changed to" << m_viewport;

        emit viewportChanged(m_viewport);
    }
}

void model::LiveCaptureModel::setDepthCameraIndex(int depthCameraIndex) {
    if (m_depthCameraIndex != depthCameraIndex)     {
        m_depthCameraIndex = depthCameraIndex;
        qInfo() << this << "Changing depth camera index to" << m_depthCameraIndex;

        emit depthCameraIndexChanged(m_depthCameraIndex);
    }
}

void model::LiveCaptureModel::setKeystoneCorrectionMode(model::LiveCaptureModel::KeystoneCorrectionMode mode) {
    if (mode != m_keystoneCorrectionMode) {
        m_keystoneCorrectionMode = mode;
        qDebug() << this << "Changing keystone correction mode to" << mode;
        emit keystoneCorrectionModeChanged(m_keystoneCorrectionMode);
    }
}

} // namespace model
} // namespace capture
