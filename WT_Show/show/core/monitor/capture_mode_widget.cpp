#include "capture_mode_widget.h"
#include "ui_capture_mode_widget.h"

#include <QDebug>
#include <QPropertyAnimation>

#include "common/utilities.h"
#include "event/change_mat_mode_event.h"

namespace capture {
namespace monitor {

const int ModeIconHeight = 56;

CaptureModeWidget::CaptureModeWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::CaptureModeWidget) {
  ui->setupUi(this);

  ui->desktopButton->setText(tr("Show desktop"));
  ui->desktopButton->setIconName("icon-monitor");

  ui->lampOnButton->setText(tr("Lamp on"));
  ui->lampOnButton->setIconName("icon-lighton");

  ui->lampOffButton->setText(tr("Lamp off"));
  ui->lampOffButton->setIconName("icon-lightoff");

  ui->reprojectionButton->setText(tr("Reprojection"));
  ui->reprojectionButton->setIconName("icon-reprojection");
}

CaptureModeWidget::~CaptureModeWidget() {}

void CaptureModeWidget::setModel(QSharedPointer<model::ApplicationStateModel> model) {
  m_model = model;
  auto liveCapture = m_model->liveCapture();

  connect(liveCapture.data(), &model::LiveCaptureModel::captureStateChanged, this,
          &CaptureModeWidget::updateEnabled);
  connect(liveCapture.data(), &model::LiveCaptureModel::videoStreamStateChanged, this,
          &CaptureModeWidget::updateEnabled);

  updateEnabled();

  connect(m_model.data(), &model::ApplicationStateModel::matModeStateChanged, this,
          &CaptureModeWidget::onMatModeStateChanged);
  onMatModeStateChanged(m_model->matModeState(), m_model->matModeState());

  connect(m_model.data(), &model::ApplicationStateModel::mainWindowLocationChanged, this,
          &CaptureModeWidget::onMatEnabledChanged);
  connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this,
          &CaptureModeWidget::onApplicationModeChanged);
  onApplicationModeChanged(m_model->mode());

  // Setup first position
  ui->ball->move(ui->ball->x(), m_model->singleScreenMode() ? ModeIconHeight : 2 * ModeIconHeight);

  if (m_model->singleScreenMode()) {
    ui->reprojectionButton->hide();
    ui->desktopButton->hide();

    ui->lampOffButton->setGeometry(0, ui->lampOnButton->y(), ui->lampOffButton->width(), ui->lampOffButton->height());
    ui->lampOnButton->setGeometry(0, ui->desktopButton->y(), ui->lampOnButton->width(), ui->lampOnButton->height());
  }
}

void CaptureModeWidget::updateEnabled() {
  bool isEnabled = !m_model->isInTransitionalMatMode();

  if (isEnabled) {
      switch(m_model->mode()) {
      case model::ApplicationStateModel::Mode::LiveCapture: {
          auto liveCapture = m_model->liveCapture();
          isEnabled = liveCapture->captureState() == model::LiveCaptureModel::CaptureState::NotCapturing &&
                      liveCapture->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running;
          break;
      }
      case model::ApplicationStateModel::Mode::Preview:
          isEnabled = m_model->mainWindowLocation() != WindowLocation::MonitorOnMat;
          break;
      case model::ApplicationStateModel::Mode::CameraFailedToStart:
      case model::ApplicationStateModel::Mode::NoCalibrationData:
      case model::ApplicationStateModel::Mode::NoVideoSource:
      case model::ApplicationStateModel::Mode::None:
          isEnabled = false;
          break;
      default:
          Q_UNREACHABLE();
      }
  }

  ui->pathWidget->setStyleSheet("background-color: #2F3438;border-radius: 23px;");
  ui->ball->setStyleSheet("background-color: #11506D;border-radius: 23px;");

  // Reset buttons UI
  ui->lampOffButton->setDisabledColor(AbstractMenuButton::defaultDisabledColor());
  ui->lampOnButton->setDisabledColor(AbstractMenuButton::defaultDisabledColor());
  ui->desktopButton->setDisabledColor(AbstractMenuButton::defaultDisabledColor());
  ui->reprojectionButton->setDisabledColor(AbstractMenuButton::defaultDisabledColor());

  ui->desktopButton->setDisabledIconName("icon-monitor-disable.png");
  ui->lampOnButton->setDisabledIconName("icon-lighton-disable.png");
  ui->lampOffButton->setDisabledIconName("icon-lightoff-disable.png");
  ui->reprojectionButton->setDisabledIconName("icon-reprojection-disable.png");

  ui->lampOffButton->setEnabled(isEnabled);
  ui->lampOnButton->setEnabled(isEnabled);
  ui->desktopButton->setEnabled(isEnabled);
  ui->reprojectionButton->setEnabled(isEnabled);

  if (isEnabled) {
      // If everything is running as expected we want to show normal icon since the background of
      // selector is blue

      ui->pathWidget->setStyleSheet("background-color: #4a4e52;border-radius: 23px;");
      ui->ball->setStyleSheet("background-color: #0096d6;border-radius: 23px;");
      switch (m_model->matModeState()) {
      case model::ApplicationStateModel::MatModeState::Desktop:
          ui->desktopButton->setDisabledColor(AbstractMenuButton::defaultCheckedColor());
          ui->desktopButton->setDisabledIconName("icon-monitor-hover.png");
          ui->desktopButton->setEnabled(false);
          break;
      case model::ApplicationStateModel::MatModeState::LampOn:
          ui->lampOnButton->setDisabledColor(AbstractMenuButton::defaultCheckedColor());
          ui->lampOnButton->setDisabledIconName("icon-lighton-hover.png");
          ui->lampOnButton->setEnabled(false);
          break;
      case model::ApplicationStateModel::MatModeState::LampOff:
          ui->lampOffButton->setDisabledColor(AbstractMenuButton::defaultCheckedColor());
          ui->lampOffButton->setDisabledIconName("icon-lightoff-hover.png");
          ui->lampOffButton->setEnabled(false);
          break;
      case model::ApplicationStateModel::MatModeState::Reprojection:
          ui->reprojectionButton->setDisabledColor(AbstractMenuButton::defaultCheckedColor());
          ui->reprojectionButton->setDisabledIconName("icon-reprojection-hover.png");
          ui->reprojectionButton->setEnabled(false);
          break;
      }
  }
}

void CaptureModeWidget::moveBallTo(int y, bool animate) {
  const QPoint finalPos(ui->ball->x(), y);
  if (animate) {
    QPropertyAnimation* animation = new QPropertyAnimation(ui->ball, "pos");
    animation->setDuration(300);
    animation->setStartValue(QPoint(ui->ball->x(), ui->ball->y()));
    animation->setEndValue(finalPos);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
  } else {
    ui->ball->move(finalPos);
  }
}

void CaptureModeWidget::onApplicationModeChanged(model::ApplicationStateModel::Mode mode) {
    int height = 238;

    if (m_model->singleScreenMode()) {
        height = 126;
    } else {
        if ( mode == model::ApplicationStateModel::Mode::Preview) {
          ui->reprojectionButton->show();
        } else {
          if (ui->ball->y() == ModeIconHeight * 3) {
            ui->ball->move(ui->ball->x(), ModeIconHeight * 2);
          }
          ui->reprojectionButton->hide();
          height -= ModeIconHeight;
        }
    }

    setHeight(height);

    update();
    if (auto widget = qobject_cast<QWidget*>(parent())) {
        widget->update();
    }

    updateEnabled();
}

void CaptureModeWidget::setHeight(int height) {
  setMinimumHeight(height);
  QRect main = geometry();
  setGeometry(main.x(), main.y(), main.width(), height);

  QRect backWidget = ui->backWidget->geometry();
  ui->backWidget->setGeometry(backWidget.x(), backWidget.y(), backWidget.width(), height);

  QRect buttonWidget = ui->buttonWidget->geometry();
  ui->buttonWidget->setGeometry(buttonWidget.x(), buttonWidget.y(), buttonWidget.width(), height);
}

void CaptureModeWidget::requestModeSwitch(event::ChangeMatModeEvent::MatMode mode) {
  common::Utilities::playSound("qrc:/Resources/production/Sounds/captureModeSelector.aif");
  auto captureModeEvent = new event::ChangeMatModeEvent(mode);
  captureModeEvent->dispatch();
}

void CaptureModeWidget::onMatModeStateChanged(model::ApplicationStateModel::MatModeState state,
                                              model::ApplicationStateModel::MatModeState oldState) {
  Q_UNUSED(state);
  ui->lampOnButton->setChecked(false);
  ui->desktopButton->setChecked(false);
  ui->lampOffButton->setChecked(false);
  ui->reprojectionButton->setChecked(false);

  // SPROUT-18441 - In case we are coming out of Reprojection mode but we are already in Live
  // capture we don't need to animate
  const auto animateBall = m_model->mode() != model::ApplicationStateModel::Mode::LiveCapture ||
                           oldState != model::ApplicationStateModel::MatModeState::Reprojection;

  // SPROUTSW-3005 - We don't need to move the indicator when cameras are starting to avoid UI glitches
  auto setIndicators = m_model->mode() == model::ApplicationStateModel::Mode::Preview ?
              true : m_model->liveCapture()->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running;

  const int singleScreenModeOffset = m_model->singleScreenMode() ? ModeIconHeight : 0;

  if (setIndicators) {
      switch (m_model->matModeState()) {
        case model::ApplicationStateModel::MatModeState::Desktop:
        case model::ApplicationStateModel::MatModeState::TransitioningToDesktop:
          moveBallTo(0, animateBall);
          ui->desktopButton->setChecked(true);
          break;
        case model::ApplicationStateModel::MatModeState::LampOn:
        case model::ApplicationStateModel::MatModeState::TransitioningToLampOn:
          moveBallTo(ModeIconHeight - singleScreenModeOffset, animateBall);
          ui->lampOnButton->setChecked(true);
          break;
        case model::ApplicationStateModel::MatModeState::LampOff:
        case model::ApplicationStateModel::MatModeState::TransitioningToLampOff:
          moveBallTo(ModeIconHeight * 2 - singleScreenModeOffset, animateBall);
          ui->lampOffButton->setChecked(true);
          break;
        case model::ApplicationStateModel::MatModeState::Reprojection:
          if (m_model->mode() == model::ApplicationStateModel::Mode::Preview) {
            moveBallTo(ModeIconHeight * 3, animateBall);
            ui->reprojectionButton->setChecked(true);
          }
          break;
        case model::ApplicationStateModel::MatModeState::TransitioningToReprojection:
          moveBallTo(ModeIconHeight * 3, animateBall);
          ui->reprojectionButton->setChecked(true);
          break;
      }
  }

  updateEnabled();
}

void CaptureModeWidget::onMatEnabledChanged(WindowLocation location) {
  if (location == WindowLocation::MonitorOnMat) {
    auto captureModeEvent = new event::ChangeMatModeEvent(event::ChangeMatModeEvent::MatMode::Desktop);
    captureModeEvent->dispatch();
  } else {
    updateEnabled();
  }
}

void CaptureModeWidget::on_desktopButton_clicked() {
  requestModeSwitch(event::ChangeMatModeEvent::MatMode::Desktop);
}

void CaptureModeWidget::on_lampOnButton_clicked() {
  requestModeSwitch(event::ChangeMatModeEvent::MatMode::LampOn);
}

void CaptureModeWidget::on_lampOffButton_clicked() {
  requestModeSwitch(event::ChangeMatModeEvent::MatMode::LampOff);
}

void CaptureModeWidget::on_reprojectionButton_clicked() {
  requestModeSwitch(event::ChangeMatModeEvent::MatMode::Reprojection);
}

} // namespace monitor
} // namespace capture
