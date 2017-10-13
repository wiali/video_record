#include "change_application_state_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

ChangeApplicationStateEvent::ChangeApplicationStateEvent(bool suspended)
    : QEvent(type())
    , m_suspended(suspended)
{ }

QEvent::Type ChangeApplicationStateEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void ChangeApplicationStateEvent::dispatch() {
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
