#include "object_segmentation_model.h"

#include <QtMath>

namespace capture {
namespace model {

ObjectSegmentationModel::ObjectSegmentationModel(QRect cropArea, float skewAngle, Type type, geometry::Geometry geometry, QObject *parent)
    : QObject(parent)    
    , m_cropArea(cropArea)
    , m_skewAngle(skewAngle)
    , m_type(type)
    , m_geometry(geometry) {
    // Needed for proper cross-thread signals
    static struct Initialize {
        Initialize() {
            qRegisterMetaType<QVector<QSharedPointer<capture::model::ObjectSegmentationModel>>>();
        }
    } initialize;
}

QRect ObjectSegmentationModel::cropArea () const
{
    return m_cropArea;
}

float ObjectSegmentationModel::skewAngle () const
{
    return m_skewAngle;
}

geometry::Geometry ObjectSegmentationModel::geometry () const
{
    return m_geometry;
}

ObjectSegmentationModel::Type ObjectSegmentationModel::type() const
{
    return m_type;
}

QRect ObjectSegmentationModel::deskewedArea ()
{
    if (!m_deskewedArea.isValid())
    {
        auto radians = qDegreesToRadians(-m_skewAngle);

        auto width = static_cast<double>(m_cropArea.width()) * std::cos(radians);
        width += static_cast<double>(m_cropArea.height()) * std::sin(radians);

        auto height = static_cast<double>(m_cropArea.height()) * std::cos(radians);
        height += static_cast<double>(m_cropArea.width()) * std::sin(radians);

        auto rotatedSize = QSize(static_cast<int>(width), static_cast<int>(height));

        auto rotatedTopLeft = m_cropArea.center();
        rotatedTopLeft -= QPoint(rotatedSize.width(), rotatedSize.height()) / 2;

        m_deskewedArea = QRect (rotatedTopLeft, rotatedSize);
    }

    return m_deskewedArea;
}

} // namespace model
} // namespace capture
