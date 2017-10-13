#include "system_event_monitor.h"

#include <QApplication>
#include <QGuiApplication>
#include <QTimer>

#include "common/utilities.h"
#include "event/change_application_state_event.h"
#include "event/start_video_streaming_event.h"
#include "event/change_mat_mode_event.h"

#ifdef Q_OS_WIN

#include "Wtsapi32.h"
#include "windows.h"

namespace capture {
namespace components {

QString sessionIdToUserName(DWORD sessionId) {
  QString userName;

  DWORD pDataSize;
  wchar_t *pData;
  if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, sessionId, WTSUserName, &pData,
                                 &pDataSize)) {
    userName = QString::fromStdWString(pData);
    WTSFreeMemory(pData);
  } else {
    qCritical() << "Failed to determine active user with" << common::Utilities::getLastWin32Error();
  }

  return userName;
}

QString activeUserName() { return sessionIdToUserName(WTSGetActiveConsoleSessionId()); }
#elif
#error Not supported on this OS
#endif

SystemEventMonitor::SystemEventMonitor(QSharedPointer<model::ApplicationStateModel> model,
                                       QSharedPointer<SingleInstance> singleInstance,
                                       QObject *parent)
    : QObject(parent),
      m_model(model),
      m_sohalSystem(new proapi::hal::System),
      m_eventHandler(new user_event_handler::EventHandler),
      m_hadDisplayOff(false) {
  auto app = qobject_cast<QGuiApplication *>(QApplication::instance());

  connect(m_sohalSystem.data(), &proapi::hal::System::powerStateSuspend, this,
          &SystemEventMonitor::onSystemSuspend);
  connect(m_sohalSystem.data(), &proapi::hal::System::powerStateLogOff, this,
          &SystemEventMonitor::onSystemSuspend);
  connect(m_sohalSystem.data(), &proapi::hal::System::powerStateShutDown, this,
          &SystemEventMonitor::onSystemSuspend);
  connect(m_sohalSystem.data(), &proapi::hal::System::powerStateEndSession, this,
          &SystemEventMonitor::onSystemSuspend);
  connect(m_sohalSystem.data(), &proapi::hal::System::sessionChanged, this,
          &SystemEventMonitor::onSessionChanged);

  connect(m_sohalSystem.data(), &proapi::hal::System::powerStateResume, this,
          &SystemEventMonitor::onSystemResume);

  connect(m_eventHandler.data(), &user_event_handler::EventHandler::displayOff, this,
          &SystemEventMonitor::onDisplayOff);
  connect(m_eventHandler.data(), &user_event_handler::EventHandler::displayOn, this,
          &SystemEventMonitor::onDisplayOn);


  connect(singleInstance.data(), &SingleInstance::receivedArguments, this,
          &SystemEventMonitor::onSystemResume);

  connect(app, &QApplication::applicationStateChanged, this,
          &SystemEventMonitor::onApplicationStateChanged);

  QCoreApplication::instance()->installEventFilter(this);

  m_currentUserName = activeUserName();

  qInfo() << this << "Current user name is" << m_currentUserName;
}

void SystemEventMonitor::onSessionChanged(proapi::hal::SessionChange sessionChange) {
  qInfo() << this << "Session change" << sessionChange.event << sessionChange.session_id;

  if (sessionChange.event == "session_unlock" ||
      // SPROUTSW-4257 - when switching user account session unlock is not sent, only console_connect
      sessionChange.event == "console_connect") {
    auto userName = sessionIdToUserName(static_cast<DWORD>(sessionChange.session_id));
    qInfo() << this << "User" << userName << "is being resumed";

    if (userName == m_currentUserName) {
      resume(false);
    }
    //make sure we update the present button state
    // Commenting out for now - if this is an issue it should be solved in event_handler library
    //onDisplayCountChanged(m_eventHandler->displayCount());

  } else if (sessionChange.event == "console_disconnect") {
    auto userName = sessionIdToUserName(static_cast<DWORD>(sessionChange.session_id));
    qInfo() << this << "User" << userName << "is being suspended";

    if (userName == m_currentUserName) {
      // Make sure that we don't continue streaming even if presentation mode is on
      m_model->setPresentationMode(false);
      onSystemSuspend();
    }
  }

  // Commenting out for now - if this is an issue it should be solved in event_handler library
  /*else if (sessionChange.event == "session_logon"){
      qDebug() << "User logon, will check monitor count.";
      onDisplayCountChanged(m_eventHandler->displayCount());
  }*/
}

void SystemEventMonitor::onDisplayOff() {
  m_hadDisplayOff = true;
  onSystemSuspend();
}

void SystemEventMonitor::onDisplayOn() {
  // SPROUT-19366 - Don't wake up if the current user is not the one that is active
  if (activeUserName() == m_currentUserName && m_hadDisplayOff) {
    resume(true);
  }
}

bool SystemEventMonitor::eventFilter(QObject *obj, QEvent *event) {
  bool processed = false;

  if (event->type() == event::ChangeApplicationStateEvent::type()) {
    auto changeApplicationStateEvent = static_cast<event::ChangeApplicationStateEvent *>(event);

    // In Presentation mode application should not be suspended
    if (changeApplicationStateEvent != nullptr) {
      m_model->setMonitorWindowMinimized(changeApplicationStateEvent->suspended());

      if (changeApplicationStateEvent->suspended()) {
        suspend(true);
      } else {
        resume(false);
      }

      processed = true;
    }
  }

  return processed ? processed : QObject::eventFilter(obj, event);
}

void SystemEventMonitor::onSystemSuspend() { suspend(false); }

void SystemEventMonitor::suspend(bool isApplicationSuspend) {
  qInfo() << this << "OnSuspend received, is already suspended?" << m_model->applicationSuspended();

  if (!m_model->applicationSuspended()) {
    m_preSuspendAppplicationMode = m_model->mode();
    m_preSuspendMatModeState = m_model->matModeState();

    // We want to continue as usual when application is presenting in live capture and system is not going to sleep
    if (!m_model->presentationMode() || !isApplicationSuspend) {
      m_model->setMode(model::ApplicationStateModel::Mode::None);
      m_model->setMatModeState(model::ApplicationStateModel::MatModeState::None);
      auto event = new event::ChangeMatModeEvent(event::ChangeMatModeEvent::MatMode::None);
      event->dispatch();
    }

    m_model->setApplicationSuspended(true);
  }
}

void SystemEventMonitor::onSystemResume() {
    resume(true);
}

void SystemEventMonitor::resume(bool changeMatModeToPreSuspendMode) {
  if (m_model->applicationSuspended()) {
    m_hadDisplayOff = false;

    m_model->setMode(m_preSuspendAppplicationMode);

    // SPROUTSW-4263 - When camera is running it's safe to transition directly to target state, otherwise we need to go full way around
    if (m_model->liveCapture()->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running) {
        m_model->setMatModeState(m_preSuspendMatModeState);
    }

    if (changeMatModeToPreSuspendMode || m_model->mode() != model::ApplicationStateModel::Mode::Preview) {
        auto event = new event::ChangeMatModeEvent(common::Utilities::MatModeStateToMatMode(m_preSuspendMatModeState));
        event->dispatch();
    }

    m_model->setApplicationSuspended(false);
  }
}

void SystemEventMonitor::onApplicationStateChanged(Qt::ApplicationState state) {
  switch (state) {
    case Qt::ApplicationInactive:
      break;
    case Qt::ApplicationActive: {
      // When application is in presentation mode and reprojecting we don't want to cycle the
      // settings
      // to avoid flickering
      // I don't this is a good solution, it will cause other issues such as SPROUT-19527
//      auto isReprojectingAndPresenting =
//          m_model->presentationMode() && m_model->mode() == model::ApplicationStateModel::Mode::Preview &&
//          m_model->matModeState() == model::ApplicationStateModel::MatModeState::Reprojection;

//      if (!isReprojectingAndPresenting) {
//        auto event = new event::ChangeMatModeEvent(MatEventModeTranslationTable[m_model->matModeState()]);
//        event->dispatch();
//      }

      break;
    }
    case Qt::ApplicationHidden:
    case Qt::ApplicationSuspended:
      // Qt::ApplicationInactive is intentionally left out since we want to keep camera running
      // while user works on
      // Desktop
      suspend(true);
      break;
  }
}

} // namespace components
} // namespace capture
