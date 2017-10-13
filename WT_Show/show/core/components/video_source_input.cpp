#include "video_source_input.h"

#include <QMetaEnum>
#include <QtConcurrentRun>
#include <QCoreApplication>
#include <video_source/nizzaexception.h>
#include <video_source/invalidconfigurationexception.h>

#include <global_utilities.h>

#include "common/utilities.h"
#include "event/start_video_streaming_event.h"
#include "event/stop_video_streaming_event.h"

namespace capture {
namespace components {

VideoSourceInput::VideoSourceInput(QSharedPointer<components::LiveVideoStreamCompositor> compositor,
                                   QSharedPointer<image_filters::ImageFilter> imageFilter,
                                   QSharedPointer<model::ApplicationStateModel> model,
                                   QObject *parent)
    : QObject(parent)
    , m_compositor(compositor)
    , m_imageFilter(imageFilter)
    , m_model(model)
{    
    connect(m_compositor->videoPipeline().data(), &video::source::VideoPipeline::stateChanged, this, &VideoSourceInput::onPipelineStateChanged);
    connect(m_compositor.data(), &LiveVideoStreamCompositor::streamFrozen, this, &VideoSourceInput::onStreamFrozen);
    connect(m_compositor.data(), &LiveVideoStreamCompositor::updated, this, &VideoSourceInput::onCompositorUpdated);

    QCoreApplication::instance()->installEventFilter(this);
}

VideoSourceInput::~VideoSourceInput() {
    m_compositor->stopFreezeDetection();

    // Make sure that video pipeline is stopped before destructing input thread to avoid
    // writing frame buffer to non-existing location
    m_compositor->videoPipeline()->stop();
}

void VideoSourceInput::onStreamFrozen(common::VideoSourceInfo videoSource)
{
    if (m_model->mode() == model::ApplicationStateModel::Mode::LiveCapture &&
        m_model->liveCapture()->captureState() == model::LiveCaptureModel::CaptureState::NotCapturing) {
        m_compositor->stopFreezeDetection();

        qWarning() << this << "Video stream" << videoSource << "seems to be frozen, restarting ...";

        m_model->setMode(model::ApplicationStateModel::Mode::None);
        m_model->setMode(model::ApplicationStateModel::Mode::LiveCapture);
    } else {
        m_compositor->restartFreezeDetectionTimers();
    }
}

void VideoSourceInput::onPipelineStateChanged()
{
    if (m_compositor->videoPipeline()->sources().count() > 0)
    {
        // Order of events is not reliable, let's just use last information we have
        auto state = m_compositor->videoPipeline()->state();

        switch (state)
        {
        case video::source::VideoPipeline::VideoPipelineState::Unintialized:
            return;
        case video::source::VideoPipeline::VideoPipelineState::Starting:
            m_model->liveCapture()->setVideoStreamState(model::LiveCaptureModel::VideoStreamState::Starting);
            return;
        case video::source::VideoPipeline::VideoPipelineState::Running:
            m_compositor->attachSourcePipelines();
            return;
        case video::source::VideoPipeline::VideoPipelineState::Stopping:
            m_compositor->detachSourcePipelines();
            return;
        case video::source::VideoPipeline::VideoPipelineState::Stopped:
            m_compositor->stopFreezeDetection();
            m_model->liveCapture()->setVideoStreamState(model::LiveCaptureModel::VideoStreamState::Stopped);
            return;
        }

        Q_UNREACHABLE();
    }    
}

void VideoSourceInput::onCompositorUpdated() {
    m_model->liveCapture()->setVideoStreamState(model::LiveCaptureModel::VideoStreamState::Running);
}

bool VideoSourceInput::eventFilter(QObject *obj, QEvent *event) {
    bool processed = false;

    if (event->type() == event::StartVideoStreamingEvent::type()) {
        if (auto startVideoStreamingEvent = static_cast<event::StartVideoStreamingEvent*>(event)) {
            start(startVideoStreamingEvent->videoSources());
            processed = true;
        }
    }
    else if (event->type() == event::StopVideoStreamingEvent::type()) {
        if (auto stopVideoStreamingEvent = static_cast<event::StopVideoStreamingEvent*>(event)) {
            stop();
            processed = true;
        }
    }

    return processed ? processed : QObject::eventFilter(obj, event);
}

void VideoSourceInput::start(QVector<common::VideoSourceInfo> videoSources)
{
    try
    {
        if (m_compositor->videoPipeline()->state() == video::source::VideoPipeline::VideoPipelineState::Running) {
            stop();
        }

        static QHash<common::VideoSourceInfo, video::source::SourcePipeline::SourcePipelineType> knownSourcesMap {
           { common::VideoSourceInfo::DownwardFacingCamera(), video::source::SourcePipeline::SourcePipelineType::DownwardFacingCamera },
           { common::VideoSourceInfo::ForwardFacingCamera(), video::source::SourcePipeline::SourcePipelineType::ForwardFacingCamera },
           { common::VideoSourceInfo::SproutCamera(), video::source::SourcePipeline::SourcePipelineType::SproutCamera },
           { common::VideoSourceInfo::PrimaryDesktop(), video::source::SourcePipeline::SourcePipelineType::PrimaryDesktop },
           { common::VideoSourceInfo::MatDesktop(), video::source::SourcePipeline::SourcePipelineType::MatDesktop },
        };

        for(auto videoSource : videoSources) {
            QSharedPointer<video::source::SourcePipeline> source;

            if (knownSourcesMap.contains(videoSource)) {
                source = m_compositor->videoPipeline()->addKnownSource(knownSourcesMap[videoSource]);
            } else {
                video::source::SourcePipelineConfig config;
                config.sourceType = video::source::SourcePipelineConfig::DirectShow;
                config.inputVideoSource.name = videoSource.name;
                config.inputVideoSource.frameRate = videoSource.frameRate;
                config.inputVideoSource.size = videoSource.resolution;

                config.keystoneCorrection.included = false;
                config.illuminationCorrection.included = false;
                config.colorCorrection.included = false;
                config.autoWhiteBalance.included = false;

                source = m_compositor->videoPipeline()->addSource(videoSource.name, config);
            }

            auto sourceConfig = source->configuration();

            // Use GPU accelerated processing
            sourceConfig.backend = video::source::SourcePipelineConfig::Backend::OpenGL;

            switch (videoSource.type)
            {
            case common::VideoSourceInfo::SourceType::Invalid:
            case common::VideoSourceInfo::SourceType::SproutCamera:
            case common::VideoSourceInfo::SourceType::Webcamera:
            case common::VideoSourceInfo::SourceType::ForwardFacingCamera:
                break;
            case common::VideoSourceInfo::SourceType::DownwardFacingCamera:
                sourceConfig.illuminationCorrection.included = true;
                sourceConfig.colorCorrection.included = true;
                break;
            case common::VideoSourceInfo::SourceType::PrimaryDesktop: {
                auto geometry = GlobalUtilities::findScreenGeometry(GlobalUtilities::ScreenType::MonitorScreen);
                sourceConfig.desktopCaptureSource.captureArea = geometry;
                break;
            }
            case common::VideoSourceInfo::SourceType::MatDesktop: {
                auto geometry = GlobalUtilities::findScreenGeometry(GlobalUtilities::ScreenType::MatScreen);
                sourceConfig.desktopCaptureSource.captureArea = geometry;
                break;
            }
            default:
                Q_UNREACHABLE();
            }

            source->setConfiguration(sourceConfig);

            m_sourceConnections << connect(source.data(), &video::source::SourcePipeline::enabledChanged, this, &VideoSourceInput::isEnabledChanged);
            m_model->liveCapture()->videoStreamSource(videoSource)->setPipelineName(source->name());

            m_needStreamedFrame << source->name();
        }

        m_compositor->videoPipeline()->start();
    }
    catch (video::source::InvalidConfigurationException& ex)
    {
        qCritical() << this <<  QString(typeid(ex).name()) << QString::fromStdString(ex.what());

        if (m_model)
        {
            stop();
            m_model->liveCapture()->setVideoStreamState(model::LiveCaptureModel::VideoStreamState::CalibrationDataMissing);
        }
    }
    catch (QException & ex)
    {
        qCritical() << this <<  QString(typeid(ex).name()) << QString::fromStdString(ex.what());

        if (m_model)
        {
            stop();
            m_model->liveCapture()->setVideoStreamState(model::LiveCaptureModel::VideoStreamState::FailedToStart);
        }
    }
}

void VideoSourceInput::stop()
{
    m_compositor->stopFreezeDetection();

    if(m_compositor->videoPipeline()->sources().empty()) {
        qWarning() << "No source available to be used";
    }
    else {
        foreach(auto connection, m_sourceConnections)
        {
            disconnect(connection);
        }

        m_sourceConnections.clear();

        m_compositor->videoPipeline()->stop();

        for(auto source : m_compositor->videoPipeline()->sources()) {
            m_compositor->videoPipeline()->removeSource(source->name());
        }

        m_model->liveCapture()->setVideoStreamState(model::LiveCaptureModel::VideoStreamState::Stopped);
    }
}

void VideoSourceInput::onAppStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive)
    {
        if(m_compositor->videoPipeline()->state() == video::source::VideoPipeline::VideoPipelineState::Stopped )
        {
            m_compositor->videoPipeline()->start();
        }
    }
    else if (state == Qt::ApplicationSuspended || state == Qt::ApplicationHidden|| state == Qt::ApplicationInactive)
    {
        if(m_compositor->videoPipeline()->state() == video::source::VideoPipeline::VideoPipelineState::Running)
        {
            m_compositor->videoPipeline()->stop();
        }
    }
}

} // namespace components
} // namespace capture

