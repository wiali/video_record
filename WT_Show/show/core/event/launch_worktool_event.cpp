#include "launch_worktool_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

LaunchWorktoolEvent::LaunchWorktoolEvent(Worktool worktool, QStringList parameters)
    : QEvent(type())
    , m_worktool(worktool)
    , m_parameters(parameters) {
    // Needed for proper cross-thread signals
    static struct Initialize {
        Initialize() {
            qRegisterMetaType<capture::event::LaunchWorktoolEvent::Worktool>();
            qRegisterMetaType<event::LaunchWorktoolEvent::Worktool>();
            qRegisterMetaType<LaunchWorktoolEvent::Worktool>();
        }
    } initialize;
}

QEvent::Type LaunchWorktoolEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void LaunchWorktoolEvent::dispatch() {
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
