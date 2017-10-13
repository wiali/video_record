#pragma once
#ifndef MAT_MODE_STATE_MACHINE_H
#define MAT_MODE_STATE_MACHINE_H

#include <atomic>

#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QRunnable>
#include <QSharedPointer>
#include <QThreadPool>

#include "event/change_mat_mode_event.h"
#include "mat_mode_state_machine_worker.h"
#include "model/application_state_model.h"

namespace capture {
namespace components {

/*!
 * \brief The MatModeStateMachine class is responsible for switching projector, touchmat and camera
 * settings according
 * to various Mat modes.
 * \details Certain mat modes requires running camera in order to properly configure the state so
 * request for state
 * \details change might be postponed until camera starts properly. Once all components are ready
 * each state request
 * \details first go to transitional state and after all changes aredone the state will toggle to
 * final state.
 * \details This class monitors occurences of ChangeMatModeEvent to start transition to target
 * state. Only single mat
 * \details mode change is being executed at the same time but it is possible to queue up multiple
 * requests for state
 * \details change.
 */
class MatModeStateMachine : public QObject {
  Q_OBJECT

 public:
  explicit MatModeStateMachine(QObject *parent = nullptr);
  virtual ~MatModeStateMachine();

  void setModel(QSharedPointer<model::ApplicationStateModel> model);

 signals:
  void transitionRequested(capture::event::ChangeMatModeEvent::MatMode mode);

 public slots:
  void abort();

 protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);
  bool checkForRunningCamera();

  QMutex m_mutex;
  QQueue<event::ChangeMatModeEvent::MatMode> m_failedTransitionsQueue;
  QScopedPointer<QThread> m_workerThread;
  QScopedPointer<MatModeStateMachineWorker> m_worker;

  QSharedPointer<model::ApplicationStateModel> m_model;

 private slots:

  void onVideoStreamStateChanged();
  void onTransitionFailed(event::ChangeMatModeEvent::MatMode mode);
  void onTransitioned(event::ChangeMatModeEvent::MatMode mode);
};

} // namespace components
} // namespace capture

#endif  // MAT_MODE_STATE_MACHINE_H
