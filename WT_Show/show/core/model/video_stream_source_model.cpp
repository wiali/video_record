#include "video_stream_source_model.h"

#include <QDebug>
#include <limits>

namespace capture {
namespace model {

VideoStreamSourceModel::VideoStreamSourceModel(common::VideoSourceInfo videoSource, QObject *parent)
    : QObject(parent)
    , m_illuminationCorrectionEnabled(false)
    , m_autoWhiteBalanceEnabled(false)
    , m_colorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode::None)
    , m_videoSource(videoSource)
    , m_skipFrameCount(-1)
    , m_frameFreezeDetectionTimeout(-1) {
    // Needed for proper cross-thread signals
    static struct Initialize {
        Initialize() {
            qRegisterMetaType<capture::model::VideoStreamSourceModel::ColorCorrectionMode>();
        }
    } initialize;
}

void model::VideoStreamSourceModel::setIlluminationCorrectionEnabled(bool isEnabled) {
    if (isEnabled != m_illuminationCorrectionEnabled) {
        m_illuminationCorrectionEnabled = isEnabled;
        emit illuminationCorrectionEnabledChanged(m_illuminationCorrectionEnabled);
    }
}

void model::VideoStreamSourceModel::setAutoWhiteBalanceEnabled(bool isEnabled) {
    if (isEnabled != m_autoWhiteBalanceEnabled) {
        m_autoWhiteBalanceEnabled = isEnabled;
        emit autoWhiteBalanceEnabledChanged(m_autoWhiteBalanceEnabled);
    }
}

void model::VideoStreamSourceModel::setColorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode mode) {
    if (mode != m_colorCorrectionMode) {
        m_colorCorrectionMode = mode;
        qDebug() << this << "Changing color correction mode to" << mode;
        emit colorCorrectionModeChanged(m_colorCorrectionMode);
    }
}

} // namespace model
} // namespace capture
