#include "forward_facing_camera_button.h"

namespace capture {
namespace monitor {

ForwardFacingCameraButton::ForwardFacingCameraButton(QWidget *parent) : CameraButton(parent) { }

void ForwardFacingCameraButton::setModel(QSharedPointer<model::ApplicationStateModel> model) {
    CameraButton::setModel(common::VideoSourceInfo::ForwardFacingCamera(), model);

    connect(m_model->liveCapture().data(), &model::LiveCaptureModel::videoStreamSourcesChanged, this, &ForwardFacingCameraButton::onVideoStreamSourcesChanged);
    onVideoStreamSourcesChanged(m_model->liveCapture()->videoStreamSources());
}

void ForwardFacingCameraButton::onVideoStreamSourcesChanged(QVector<QSharedPointer<model::VideoStreamSourceModel>> videoStreamSources) {
    QSharedPointer<model::VideoStreamSourceModel> firstWebcamSource;

    for (auto streamSource : videoStreamSources) {
        if (streamSource->videoSource().type == common::VideoSourceInfo::SourceType::Webcamera) {
            firstWebcamSource = streamSource;
            break;
        }
    }

    auto videoSources = m_videoSources;

    for (int i = videoSources.length() - 1; i >= 0; i--) {
        if (videoSources[i].type == common::VideoSourceInfo::SourceType::Webcamera) {
            videoSources.removeAt(i);
        }
    }

    if (!firstWebcamSource.isNull()) {
        videoSources << firstWebcamSource->videoSource();
    }

    if (videoSources != m_videoSources) {
        m_videoSources = videoSources;

        onSelectedVideoStreamSourcesChanged(m_model->liveCapture()->selectedVideoStreamSources());
    }
}

} // namespace monitor
} // namespace capture
