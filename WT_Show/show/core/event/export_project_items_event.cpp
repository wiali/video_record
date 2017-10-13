#include "export_project_items_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

ExportProjectItemsEvent::ExportProjectItemsEvent(ExportProjectItemsEvent::Format format, QString location,
                                                 QVector<QSharedPointer<model::ExportImageModel>> items, bool single)
    : QEvent(type())
    , m_format(format)
    , m_location(location)
    , m_items(items)
    , m_single(single) {
    // Needed for proper cross-thread signals
    static struct Initialize {
        Initialize() {
            qRegisterMetaType<capture::event::ExportProjectItemsEvent::Format>();
            qRegisterMetaType<event::ExportProjectItemsEvent::Format>();
            qRegisterMetaType<ExportProjectItemsEvent::Format>();
        }
    } initialize;
}

QEvent::Type ExportProjectItemsEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void ExportProjectItemsEvent::dispatch() {
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
