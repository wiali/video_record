#include "change_invalid_project_name_visibility_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

ChangeInvalidProjectNameVisibilityEvent::ChangeInvalidProjectNameVisibilityEvent(bool visible, QPoint globalPosition)
    : QEvent(type())
    , m_globalPosition(globalPosition)
    , m_visible(visible)
{ }

QEvent::Type ChangeInvalidProjectNameVisibilityEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void ChangeInvalidProjectNameVisibilityEvent::dispatch() {
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
