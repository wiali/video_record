#include "export_projects_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

ExportProjectsEvent::ExportProjectsEvent(QVector<QSharedPointer<StageProject> > projects)
    : QEvent(type())
    , m_projects(projects)
{ }

QEvent::Type ExportProjectsEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void ExportProjectsEvent::dispatch() {
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
