#pragma once
#ifndef MATMODESTATEMACHINEWORKER_H
#define MATMODESTATEMACHINEWORKER_H

#include <QObject>
#include <QSharedPointer>

#include "event/change_mat_mode_event.h"
#include "model/application_state_model.h"

namespace capture {
namespace components {

class MatModeStateMachineWorker : public QObject {
  Q_OBJECT
 public:
  explicit MatModeStateMachineWorker(QObject *parent = 0);

  void setModel(QSharedPointer<model::ApplicationStateModel> model);
 signals:

  void transitionFailed(capture::event::ChangeMatModeEvent::MatMode mode);
  void transitioned(capture::event::ChangeMatModeEvent::MatMode mode);

 public slots:

  void transitionTo(capture::event::ChangeMatModeEvent::MatMode mode);

 private:
  bool transitionToDesktop();
  bool transitionToLampOff();
  bool transitionToLampOn();
  bool transitionToFlash();
  bool transitionToReprojection();
  bool transitionToNone();

  bool checkForRunningCamera();
  QSharedPointer<model::VideoStreamSourceModel> downwardFacingSourceModel();

  QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace components
} // namespace capture

#endif  // MATMODESTATEMACHINEWORKER_H
