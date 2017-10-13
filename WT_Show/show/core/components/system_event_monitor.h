#pragma once
#ifndef SYSTEMEVENTMONITOR_H
#define SYSTEMEVENTMONITOR_H

#include <QObject>
#include <QSharedPointer>

#include <hal/system.h>
#include <single_instance.h>

#include <user_event_handler.h>

#include "model/application_state_model.h"

namespace capture {
namespace components {

/*!
 * \brief The SystemEventMonitor class is responsible for monitoring system events for sleep/wakeup and setting correct application state based on these inputs.
 * \details This class monitors ChangeApplicationStateEvent for additional events (e.g. minimizing the main window).
 */
class SystemEventMonitor : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief SystemEventMonitor constructor.
     * \param model Model object.
     * \param parent Parent object.
     */
    explicit SystemEventMonitor(QSharedPointer<model::ApplicationStateModel> model,
                                QSharedPointer<SingleInstance> singleInstance,
                                QObject *parent = 0);
private slots:

    void onSystemSuspend();    
    void onApplicationStateChanged(Qt::ApplicationState state);
    void onDisplayOff();
    void onDisplayOn();
    void onSessionChanged(proapi::hal::SessionChange sessionChange);
    void onSystemResume();

protected:

    void suspend(bool isApplicationSuspend);
    void resume(bool changeMatModeToPreSuspendMode);
    virtual bool eventFilter(QObject *obj, QEvent *event);

    QSharedPointer<model::ApplicationStateModel> m_model;
    QScopedPointer<proapi::hal::System> m_sohalSystem;
    QScopedPointer<user_event_handler::EventHandler> m_eventHandler;
    bool m_hadDisplayOff;
    model::ApplicationStateModel::MatModeState m_preSuspendMatModeState;
    model::ApplicationStateModel::Mode m_preSuspendAppplicationMode;
    QString m_currentUserName;
};

} // namespace components
} // namespace capture

#endif // SYSTEMEVENTMONITOR_H
