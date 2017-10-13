#include "camera_button.h"

#include "event/start_video_streaming_event.h"
#include "event/change_mat_mode_event.h"

namespace capture {
namespace monitor {

CameraButton::CameraButton(QWidget *parent)
    : RightMenuButton(parent)
{
    setCheckable(true);
    connect(this, &CameraButton::clicked, this, &CameraButton::onButtonClicked);
}

void CameraButton::setModel(const common::VideoSourceInfo &videoSource,
                            QSharedPointer<model::ApplicationStateModel> model) {
    QVector<common::VideoSourceInfo> videoSources;
    videoSources << videoSource;

    setModel(videoSources, model);
}

void CameraButton::setModel(QVector<common::VideoSourceInfo> videoSources,
                            QSharedPointer<model::ApplicationStateModel> model) {
  m_model = model;
  m_videoSources = videoSources;
  auto liveCapture = m_model->liveCapture();

  connect(liveCapture.data(), &model::LiveCaptureModel::selectedVideoStreamSourcesChanged, this, &CameraButton::onSelectedVideoStreamSourcesChanged);
  onSelectedVideoStreamSourcesChanged(liveCapture->selectedVideoStreamSources());

  connect(liveCapture.data(), &model::LiveCaptureModel::videoStreamSourcesChanged, this, &CameraButton::updateDisabledState);
  connect(liveCapture.data(), &model::LiveCaptureModel::captureStateChanged, this, &CameraButton::updateDisabledState);
  connect(liveCapture.data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &CameraButton::updateDisabledState);
  connect(m_model.data(), &model::ApplicationStateModel::matModeStateChanged, this, &CameraButton::updateDisabledState);

  updateDisabledState();
}

void CameraButton::updateStyle()
{
    QString isCheckString = "";
    if (m_checked)
    {
        isCheckString = "-checked";
    }

    if (!m_enabled)
    {
        m_stateTextColor = isThisLastSource() ? defaultCheckedColor() : disabledColor();
        m_stateIcon =  QImage(":/Resources/production/" + m_iconName + isCheckString + (isThisLastSource() ? "-norm.png" : "-disable.png"));
    }
    else if (m_down)
    {
        m_stateTextColor = defaultCheckedColor();
        m_stateIcon =  QImage(":/Resources/production/" + m_iconName + isCheckString + "-press.png");
    }
    else if (m_hovering)
    {
        m_stateTextColor = m_checked?defaultCheckedColor():QColor("#ffffff");
        m_stateIcon =  QImage(":/Resources/production/" + m_iconName + isCheckString + "-hover.png");
    }
    else
    {
        m_stateTextColor = m_checked?defaultCheckedColor():normalColor();
        m_stateIcon =  QImage(":/Resources/production/" + m_iconName + isCheckString + "-norm.png");
    }

    update();
}

void CameraButton::onSelectedVideoStreamSourcesChanged(QVector<common::VideoSourceInfo> selectedCameras) {
    auto isSelected = false;

    for(auto type : m_videoSources) {
        isSelected = selectedCameras.contains(type) ? true : isSelected;
    }

    setChecked(isSelected);
    updateDisabledState();
}

bool CameraButton::isThisLastSource() {
    bool result = false;
    auto liveCapture = m_model->liveCapture();

    if (liveCapture->selectedVideoStreamSources().count() == 1) {
        for(auto type : m_videoSources) {
            result = liveCapture->selectedVideoStreamSources().first() == type ? true : result;
        }
    }

    return result;
}


void CameraButton::updateDisabledState() {
  auto liveCapture = m_model->liveCapture();
  auto isAnyCameraStarting = liveCapture->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Starting;
  auto isThisSourceAvailable = false;

  for(auto type : m_videoSources) {
      isThisSourceAvailable = liveCapture->videoStreamSource(type).isNull() ? isThisSourceAvailable : true;
  }

  auto isEnabled = liveCapture->captureState() == model::LiveCaptureModel::CaptureState::NotCapturing &&
                   isThisSourceAvailable &&
                   !m_model->isInTransitionalMatMode() && !isAnyCameraStarting && !isThisLastSource();

  setEnabled(isEnabled);
}

void CameraButton::onButtonClicked() {
  auto cameras = m_model->liveCapture()->selectedVideoStreamSources();

  for(auto type : m_videoSources) {
      auto index = cameras.indexOf(type);

      if (index >= 0 && cameras.count() > 0) {
          cameras.remove(index);
      } else if (!m_model->liveCapture()->videoStreamSource(type).isNull()) {
          cameras << type;
      }
      if(!m_model->picInPicMode())
      {
          if(!m_model->liveCapture()->videoStreamSource(type).isNull())
          {
              cameras.clear();
              cameras << type;
          }
      }
  }

  selectCameras(cameras);
}

void CameraButton::selectCameras(QVector<common::VideoSourceInfo> cameras) {
  m_model->liveCapture()->setSelectedVideoStreamSources(cameras);

  if (m_model->mainWindowLocation() == model::ApplicationStateModel::MainWindowLocation::None)
  {
      auto captureModeEvent = new event::ChangeMatModeEvent(event::ChangeMatModeEvent::MatMode::LampOff);
      captureModeEvent->dispatch();
  }
}

} // namespace monitor
} // namespace capture
