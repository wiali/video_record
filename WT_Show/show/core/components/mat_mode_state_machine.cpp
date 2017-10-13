#include "mat_mode_state_machine.h"

#include <QCoreApplication>
#include <QDebug>
#include <QQueue>
#include <QTimer>
#include <QtConcurrentRun>

#include "common/utilities.h"

namespace capture {
namespace components {

MatModeStateMachine::MatModeStateMachine(QObject *parent)
    : QObject(parent),
      m_worker(new MatModeStateMachineWorker),
      m_workerThread(new QThread) {
  QCoreApplication::instance()->installEventFilter(this);
  m_worker->moveToThread(m_workerThread.data());

  connect(this, &MatModeStateMachine::transitionRequested, m_worker.data(),
          &MatModeStateMachineWorker::transitionTo);
  connect(m_worker.data(), &MatModeStateMachineWorker::transitionFailed, this,
          &MatModeStateMachine::onTransitionFailed);
  connect(m_worker.data(), &MatModeStateMachineWorker::transitioned, this,
          &MatModeStateMachine::onTransitioned);

  m_workerThread->start();
}

void MatModeStateMachine::onTransitionFailed(event::ChangeMatModeEvent::MatMode mode) {
    QMutexLocker locker(&m_mutex);
    m_failedTransitionsQueue.enqueue(mode);

    qInfo() << this << "Transition failed, updated queue" << m_failedTransitionsQueue;
}

void MatModeStateMachine::onTransitioned(event::ChangeMatModeEvent::MatMode mode) {
    Q_UNUSED(mode);
    QMutexLocker locker(&m_mutex);
    m_failedTransitionsQueue.clear();
}

void MatModeStateMachine::setModel(QSharedPointer<model::ApplicationStateModel> model) {
  m_model = model;
  m_worker->setModel(model);

  auto liveCapture = m_model->liveCapture();

  connect(liveCapture.data(), &model::LiveCaptureModel::videoStreamStateChanged, this,
          &MatModeStateMachine::onVideoStreamStateChanged);
}

void MatModeStateMachine::abort() {
  disconnect(m_worker.data());

  if (m_workerThread->isRunning()) {
    m_workerThread->quit();
    m_workerThread->wait();
  }
}

MatModeStateMachine::~MatModeStateMachine() {
  abort();  
}

bool MatModeStateMachine::eventFilter(QObject *obj, QEvent *event) {
  bool processed = false;

  if (event->type() == event::ChangeMatModeEvent::type()) {
    auto changeMatModeEvent = static_cast<event::ChangeMatModeEvent *>(event);

    if (changeMatModeEvent != nullptr && changeMatModeEvent->mode() != event::ChangeMatModeEvent::None) {
      emit transitionRequested(changeMatModeEvent->mode());
      processed = true;
    }
  }

  return processed ? processed : QObject::eventFilter(obj, event);
}

void MatModeStateMachine::onVideoStreamStateChanged() {
  if (m_model->liveCapture()->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running) {
    QMutexLocker locker(&m_mutex);

    if (!m_failedTransitionsQueue.isEmpty()) {
      auto head = m_failedTransitionsQueue.dequeue();
      m_failedTransitionsQueue.clear();

      emit transitionRequested(head);
    }
  }
}

} // namespace components
} // namespace capture

