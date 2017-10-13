#pragma once
#ifndef SEGMENTOBJECTEVENT_H
#define SEGMENTOBJECTEVENT_H

#include <QEvent>
#include <QSharedPointer>

#include "model/camera_item_metadata.h"

namespace capture {
namespace event {

class SegmentObjectEvent : public QEvent
{
public:
    explicit SegmentObjectEvent(QSharedPointer<model::CameraItemMetadata> model);

    inline QSharedPointer<model::CameraItemMetadata> model() const { return m_model; }

    static QEvent::Type type();
    void dispatch();

private:
    QSharedPointer<model::CameraItemMetadata> m_model;
};

} // namespace event
} // namespace capture

#endif // SEGMENTOBJECTEVENT_H
