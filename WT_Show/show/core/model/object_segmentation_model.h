#pragma once
#ifndef OBJECTSEGMENTATIONMODEL_H
#define OBJECTSEGMENTATIONMODEL_H

#include <QObject>
#include <QRect>

#include "geometry.h"

namespace capture {
namespace model {

class ObjectSegmentationModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRect cropArea READ cropArea CONSTANT)
    Q_PROPERTY(float skewAngle READ skewAngle CONSTANT)
    Q_PROPERTY(ObjectSegmentationModel::Type type READ type CONSTANT)
    Q_PROPERTY(QRect deskewedArea READ deskewedArea CONSTANT)
    Q_PROPERTY(geometry::Geometry geometry READ geometry CONSTANT)

public:
    enum class Type
    {
        FlatRectangle,
        FlatNonRectangle,
        ThreeDObject
    };

    Q_ENUM(Type)

    explicit ObjectSegmentationModel(QRect cropArea,
                                     float skewAngle,
                                     ObjectSegmentationModel::Type type,
                                     geometry::Geometry geometry,
                                     QObject *parent = 0);

    QRect deskewedArea ();
    QRect cropArea () const;
    float skewAngle () const;
    ObjectSegmentationModel::Type type() const;
    geometry::Geometry geometry() const;

private:

    QRect m_deskewedArea;
    QRect m_cropArea;
    float m_skewAngle;
    ObjectSegmentationModel::Type m_type;
    geometry::Geometry m_geometry;
};

} // namespace model
} // namespace capture

#endif // OBJECTSEGMENTATIONMODEL_H
