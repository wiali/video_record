#include "mat_mode_state_machine_worker.h"

#include <QDebug>

#include "common/utilities.h"

namespace capture {
namespace components {

MatModeStateMachineWorker::MatModeStateMachineWorker(QObject *parent) : QObject(parent) {}

void MatModeStateMachineWorker::setModel(QSharedPointer<model::ApplicationStateModel> model) {
  m_model = model;
}

void MatModeStateMachineWorker::transitionTo(event::ChangeMatModeEvent::MatMode mode) {
  auto liveCapture = m_model->liveCapture();
  auto previousMatMode = m_model->matModeState();
  bool success = false;

  qInfo() << this << "Trying to transition from" << previousMatMode << "to" << mode;

  switch (mode) {
    case event::ChangeMatModeEvent::MatMode::Desktop:
      success = transitionToDesktop();
      break;
    case event::ChangeMatModeEvent::MatMode::LampOff:
      success = transitionToLampOff();
      break;
    case event::ChangeMatModeEvent::MatMode::LampOn:
      success = transitionToLampOn();
      break;
    case event::ChangeMatModeEvent::MatMode::Flash:
      success = transitionToFlash();
      break;
    case event::ChangeMatModeEvent::MatMode::Reprojection:
      success = transitionToReprojection();
      break;
    case event::ChangeMatModeEvent::MatMode::None:
      success = transitionToNone();
      break;
  }

  if (!success) {
    qInfo() << this << "Failed to transition to" << mode << ", reverting back to previous mode" << previousMatMode;
    m_model->setMatModeState(previousMatMode);
    emit transitionFailed(mode);
  } else {
    qInfo() << this << "Transitioned to" << mode;
    emit transitioned(mode);
  }
}

bool MatModeStateMachineWorker::transitionToReprojection() {
  m_model->setMatModeState(model::ApplicationStateModel::MatModeState::TransitioningToReprojection);
  // This is special post-capture mode so we don't need to wait for camera
  m_model->setMatModeState(model::ApplicationStateModel::MatModeState::Reprojection);

  return true;
}

bool MatModeStateMachineWorker::transitionToNone() {
  m_model->setMatModeState(model::ApplicationStateModel::MatModeState::TransitioningToNone);
  m_model->setMatModeState(model::ApplicationStateModel::MatModeState::None);
  return true;
}

bool MatModeStateMachineWorker::checkForRunningCamera() {
  auto result = false;

  switch (m_model->mode()) {
    case model::ApplicationStateModel::Mode::Preview:
      result = true;
      break;
    case model::ApplicationStateModel::Mode::CameraFailedToStart:
    case model::ApplicationStateModel::Mode::NoCalibrationData:
    case model::ApplicationStateModel::Mode::LiveCapture: {
      // SPROUT-17897 - We need to make sure that projector/camera will be setup correctly if
      // calibration data is missing
      auto liveCapture = m_model->liveCapture();

      result = liveCapture->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running;
      break;
    }
    case model::ApplicationStateModel::Mode::NoVideoSource:
    case model::ApplicationStateModel::Mode::None:
      result = false;
      break;
  default:
      Q_UNREACHABLE();
  }

  return result;
}

QSharedPointer<model::VideoStreamSourceModel> MatModeStateMachineWorker::downwardFacingSourceModel() {
    return m_model->liveCapture()->videoStreamSource(common::VideoSourceInfo::DownwardFacingCamera());
}

bool MatModeStateMachineWorker::transitionToDesktop() {
  m_model->setMatModeState(model::ApplicationStateModel::MatModeState::TransitioningToDesktop);

  bool transitioned = false;
  auto liveCapture = m_model->liveCapture();    

  if (checkForRunningCamera()) {
    m_model->setMatModeState(model::ApplicationStateModel::Desktop);
    transitioned = true;
  }

  return transitioned;
}

bool MatModeStateMachineWorker::transitionToLampOff() {
  m_model->setMatModeState(model::ApplicationStateModel::MatModeState::TransitioningToLampOff);
  bool transitioned = false;

  // Lamp Off is used in two contexts - with projector on and black background window
  // (DownwardFacingCamera) and with projector off (either turned off by user action or in
  // ForwardFacingCamera or Preview). Because of this we don't manipulate projector state here  

  if (checkForRunningCamera()) {
    m_model->setMatModeState(model::ApplicationStateModel::MatModeState::LampOff);
    transitioned = true;
  }

  return transitioned;
}

bool MatModeStateMachineWorker::transitionToLampOn() {
  m_model->setMatModeState(model::ApplicationStateModel::MatModeState::TransitioningToLampOn);
  bool transitioned = false;    

  if (checkForRunningCamera()) {
    m_model->setMatModeState(model::ApplicationStateModel::MatModeState::LampOn);
    transitioned = true;
  }

  return transitioned;
}

bool MatModeStateMachineWorker::transitionToFlash() {
  m_model->setMatModeState(model::ApplicationStateModel::MatModeState::TransitioningToFlash);
  bool transitioned = false;
  auto liveCapture = m_model->liveCapture();

  if (checkForRunningCamera()) {
    m_model->setMatModeState(model::ApplicationStateModel::MatModeState::Flash);

    transitioned = true;
  }

  return transitioned;
}

} // namespace components
} // namespace capture

