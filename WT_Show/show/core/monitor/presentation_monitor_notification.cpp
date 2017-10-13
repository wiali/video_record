#include "presentation_monitor_notification.h"

#include <QGuiApplication>

#include <global_utilities.h>

#include "common/utilities.h"
#include "global_utilities.h"

namespace capture {
namespace monitor {

PresentationMonitorNotification::PresentationMonitorNotification(QWidget *parent)
    : FadeOutNotification(parent)
    , m_presentationMonitorConnected(false)
    , m_eventHandler(new user_event_handler::EventHandler) {
    auto settings = GlobalUtilities::applicationSettings("presentation_monitor_notification");
    setDuration(settings->value("fade_out_timeout_ms", 3000).toInt());
    setMinimumSize(400, minimumHeight());

    m_presentationMonitorConnected = GlobalUtilities::findScreen(GlobalUtilities::PresentScreen) != nullptr;

    if (settings->value("show_notification", true).toBool()) {
        connect(m_eventHandler.data(), &user_event_handler::EventHandler::displayCountChanged, this, &PresentationMonitorNotification::onDisplayCountChanged);
    }
}

void PresentationMonitorNotification::onDisplayCountChanged(int displayCount) {
    bool isMonitorConnected = displayCount > 2;

    qDebug() << this << "Screen count change, 3rd monitor is connected?" << isMonitorConnected <<
                "previous state" << m_presentationMonitorConnected;

    if (isMonitorConnected != m_presentationMonitorConnected) {
        // There was a change, show notification

        setText(isMonitorConnected ? tr("Presentation screen connected")
                                   : tr("Presentation screen disconnected"));

        m_fadeOutAnimation->stop();
        m_fadeOutAnimation->start();
    }

    m_presentationMonitorConnected = isMonitorConnected;
}

} // namespace monitor
} // namespace capture
