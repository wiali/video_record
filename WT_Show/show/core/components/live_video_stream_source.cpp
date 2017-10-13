#include "live_video_stream_source.h"

namespace capture {
namespace components {

LiveVideoStreamSource::LiveVideoStreamSource(QSharedPointer<model::VideoStreamSourceModel> model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_frameCount(0)
{
    connect(model.data(), &model::VideoStreamSourceModel::illuminationCorrectionEnabledChanged, this, &LiveVideoStreamSource::onIlluminationCorrectionEnabledChanged);
    onIlluminationCorrectionEnabledChanged(model->illuminationCorrectionEnabled());

    connect(model.data(), &model::VideoStreamSourceModel::autoWhiteBalanceEnabledChanged, this, &LiveVideoStreamSource::onAutoWhiteBalanceEnabledChanged);
    onAutoWhiteBalanceEnabledChanged(model->autoWhiteBalanceEnabled());

    connect(model.data(), &model::VideoStreamSourceModel::colorCorrectionModeChanged, this, &LiveVideoStreamSource::onColorCorrectionModeChanged);
    onColorCorrectionModeChanged(model->colorCorrectionMode());

    connect(&m_freezeDetectionTimer, &QTimer::timeout, this, &LiveVideoStreamSource::onFreezeDetectionTimerTimeout);
}

QSharedPointer<QOpenGLFramebufferObject> LiveVideoStreamSource::frameBuffer() const {
    const auto context = QOpenGLContext::currentContext();
    return m_frameBuffers.contains(context) ? m_frameBuffers[context] : QSharedPointer<QOpenGLFramebufferObject>();
}

QSize LiveVideoStreamSource::frameSize() const { return m_frameSize; }

void LiveVideoStreamSource::restartFreezeDetectionTimer()
{
    m_freezeDetectionTimer.stop();

    if (m_model->frameFreezeDetectionTimeout() > 0) {
        m_freezeDetectionTimer.start(m_model->frameFreezeDetectionTimeout());
    }
}

void LiveVideoStreamSource::stopFreezeDetection()
{
    m_freezeDetectionTimer.stop();
}

void LiveVideoStreamSource::onFreezeDetectionTimerTimeout()
{
    m_freezeDetectionTimer.stop();

    emit streamFrozen(m_model->videoSource());
}

void LiveVideoStreamSource::onFrameReady(int sourceIndex, const QSize &size) {
    Q_UNUSED(sourceIndex);

    if (m_sourcePipeline) {
        m_frameSize = size;

        if (m_frameCount > m_model->skipFrameCount())
        {
            m_frameReadyForUpdate = true;
            emit updated(m_model->pipelineName());
        }
        else
        {
            qDebug() << this << "Waiting for" << m_model->videoSource() << "to provide at least" << m_model->skipFrameCount()
                     << "frames, so far have" << m_frameCount;
            m_frameCount++;
        }

        restartFreezeDetectionTimer();
    }
}

void LiveVideoStreamSource::updateFrameBuffer() {
    if (m_sourcePipeline) {
        const auto context = QOpenGLContext::currentContext();

        if (m_frameSize.isValid() && !m_frameBuffers.contains(context)) {
            qDebug() << this << "Creating framebuffer for source" << m_model->videoSource() << "with frame size" << m_frameSize;

            m_frameBuffers.insert(context, QSharedPointer<QOpenGLFramebufferObject>::create(m_frameSize));
        }

        if (m_frameBuffers.contains(context) && m_frameReadyForUpdate) {
            m_sourcePipeline->combinedOpenGLFilter()->convert(m_frameBuffers[context].data());
            //m_frameReadyForUpdate = false;
        }
    }
}

void LiveVideoStreamSource::setSourcePipeline(QSharedPointer<video::source::SourcePipeline> sourcePipeline) {
    m_sourcePipeline = sourcePipeline;
    m_frameCount = 0;

    disconnect(m_sourceConnection);

    if (sourcePipeline) {
        m_sourceConnection = connect(m_sourcePipeline->combinedOpenGLFilter(), &video::source::CombinedOpenGLFilter::frameReady,
                                      this, &LiveVideoStreamSource::onFrameReady);

        if (m_sourcePipeline->colorCorrection())
        {
            if (m_sourcePipeline->colorCorrection()->enabled())
            {
                switch(m_sourcePipeline->colorCorrection()->mode()) {
                case video::source::ColorCorrectionFilter::Mode::LampOff:
                    m_model->setColorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode::LampOff);
                    break;
                case video::source::ColorCorrectionFilter::Mode::LampOn:
                    m_model->setColorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode::LampOn);
                    break;
                }
            }
            else
            {
                m_model->setColorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode::None);
            }
        }
        else
        {
            m_model->setColorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode::None);
        }

        m_model->setIlluminationCorrectionEnabled(m_sourcePipeline->illuminationCorrection() && m_sourcePipeline->illuminationCorrection()->enabled());
        m_model->setAutoWhiteBalanceEnabled(m_sourcePipeline->illuminationCorrection() && m_sourcePipeline->illuminationCorrection()->autoWhiteBalance());
    }
}

void LiveVideoStreamSource::onIlluminationCorrectionEnabledChanged(bool isEnabled)
{
    if (m_sourcePipeline && m_sourcePipeline->illuminationCorrection())
    {
        m_sourcePipeline->illuminationCorrection()->setEnabled(isEnabled);
    }
}

void LiveVideoStreamSource::onAutoWhiteBalanceEnabledChanged(bool isEnabled)
{
    if (m_sourcePipeline && m_sourcePipeline->illuminationCorrection())
    {
        m_sourcePipeline->illuminationCorrection()->setAutoWhiteBalance(isEnabled);
    }
}

void LiveVideoStreamSource::onColorCorrectionModeChanged(model::VideoStreamSourceModel::ColorCorrectionMode mode)
{
    if (m_sourcePipeline && m_sourcePipeline->colorCorrection())
    {
        switch(mode) {
        case model::VideoStreamSourceModel::ColorCorrectionMode::None:
            m_sourcePipeline->colorCorrection()->setEnabled(false);
            break;
        case model::VideoStreamSourceModel::ColorCorrectionMode::LampOff:
            m_sourcePipeline->colorCorrection()->setEnabled(true);
            m_sourcePipeline->colorCorrection()->setMode(video::source::ColorCorrectionFilter::Mode::LampOff);
            break;
        case model::VideoStreamSourceModel::ColorCorrectionMode::LampOn:
            m_sourcePipeline->colorCorrection()->setEnabled(true);
            m_sourcePipeline->colorCorrection()->setMode(video::source::ColorCorrectionFilter::Mode::LampOn);
            break;
        }
    }
}

} // namespace components
} // namespace capture
