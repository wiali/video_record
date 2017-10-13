#include "no_calibration_data_form.h"
#include "ui_no_calibration_data_form.h"

#include <global_utilities.h>

#include "common/utilities.h"
#include "event/start_video_streaming_event.h"
#include "event/launch_worktool_event.h"

namespace capture {
namespace monitor {

NoCalibrationDataForm::NoCalibrationDataForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NoCalibrationDataForm)
{
    ui->setupUi(this);

    m_retryTimer.setInterval(GlobalUtilities::applicationSettings("downward_facing_camera")->value("no_calibration_data_timeout_ms", 3000).toInt());

    connect(&m_retryTimer, &QTimer::timeout, this, &NoCalibrationDataForm::onRetryTimeout);
}

void NoCalibrationDataForm::onRetryTimeout()
{
    auto monitorWindow = common::Utilities::getMonitorWindow();

    if(monitorWindow && !monitorWindow->isMinimized())
    {
        qDebug() << "Retrying to open camera";

        auto startStreamingEvent = new event::StartVideoStreamingEvent(m_model->liveCapture()->selectedVideoStreamSources());
        startStreamingEvent->dispatch();
    }
}

void NoCalibrationDataForm::onOpenControlPanelClicked()
{
    auto launchWorktoolEvent = new event::LaunchWorktoolEvent(event::LaunchWorktoolEvent::Worktool::Control);
    launchWorktoolEvent->dispatch();

    m_retryTimer.start();
}

void NoCalibrationDataForm::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;
    auto liveCapture = m_model->liveCapture();

    connect(liveCapture.data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &NoCalibrationDataForm::onVideoStreamStateChanged);
}

NoCalibrationDataForm::~NoCalibrationDataForm()
{
    delete ui;
}

void NoCalibrationDataForm::onVideoStreamStateChanged(model::LiveCaptureModel::VideoStreamState state)
{
    if (state == model::LiveCaptureModel::VideoStreamState::Running && m_retryTimer.isActive())
    {
        m_model->setMode(model::ApplicationStateModel::Mode::LiveCapture);

        m_retryTimer.stop();
    }
}

} // namespace monitor
} // namespace capture
