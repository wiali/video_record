#include "clean_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

CleanEvent::CleanEvent()
    : QEvent(type())
{
}

QEvent::Type CleanEvent::type()
{
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void CleanEvent::dispatch()
{
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
