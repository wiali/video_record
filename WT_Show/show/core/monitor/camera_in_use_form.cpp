#include "camera_in_use_form.h"
#include "ui_camera_in_use_form.h"

#include <video_source/sourcepipeline.h>

#include "event/start_video_streaming_event.h"

namespace capture {
namespace monitor {

CameraInUseForm::CameraInUseForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CameraInUseForm)
{
    ui->setupUi(this);
}

void CameraInUseForm::onRetryButtonReleased()
{
    auto startStreamingEvent = new event::StartVideoStreamingEvent(m_model->liveCapture()->selectedVideoStreamSources());
    startStreamingEvent->dispatch();
}

void CameraInUseForm::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;
}

} // namespace monitor
} // namespace capture
