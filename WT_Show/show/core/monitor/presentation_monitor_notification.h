#pragma once
#ifndef PRESENTATION_MONITOR_NOTIFICATION_H
#define PRESENTATION_MONITOR_NOTIFICATION_H

#include <QWidget>
#include <QList>

#include <user_event_handler.h>

#include "fade_out_notification.h"

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class PresentationMonitorNotification : public FadeOutNotification
{
    Q_OBJECT
public:
    explicit PresentationMonitorNotification(QWidget *parent = 0);

private slots:
    void onDisplayCountChanged(int displayCount);

private:
    bool m_presentationMonitorConnected;
    QScopedPointer<user_event_handler::EventHandler> m_eventHandler;
};

} // namespace monitor
} // namespace capture

#endif  // PRESENTATION_MONITOR_NOTIFICATION_H
