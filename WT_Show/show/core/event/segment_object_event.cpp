#include "segment_object_event.h"

#include <QCoreApplication>

namespace capture {
namespace event {

SegmentObjectEvent::SegmentObjectEvent(QSharedPointer<model::CameraItemMetadata> model)
    : QEvent(type())
    , m_model(model)
{ }

QEvent::Type SegmentObjectEvent::type() {
    static int type = QEvent::registerEventType();
    return static_cast<QEvent::Type>(type);
}

void SegmentObjectEvent::dispatch() {
    QCoreApplication::postEvent(QCoreApplication::instance(), this);
}

} // namespace event
} // namespace capture
