#include "change_mat_mode_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

ChangeMatModeEvent::ChangeMatModeEvent(MatMode mode)
    : QEvent(type())
    , m_mode(mode) {
    // Needed for proper cross-thread signals
    static struct Initialize {
        Initialize() {
            qRegisterMetaType<capture::event::ChangeMatModeEvent::MatMode>();
        }
    } initialize;
}

QEvent::Type ChangeMatModeEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void ChangeMatModeEvent::dispatch() {
    qDebug() << "Dispatching event" << typeid(*this).name() << "mode" << m_mode;
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
