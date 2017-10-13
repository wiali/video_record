#include "segmentation_collection_model.h"

namespace capture {
namespace model {

SegmentationCollectionModel::SegmentationCollectionModel(QObject *parent)
    : QObject(parent)
{ }

QVector<QSharedPointer<ObjectSegmentationModel>> SegmentationCollectionModel::objects() const
{
    return m_objects;
}

void SegmentationCollectionModel::setObjects(QVector<QSharedPointer<ObjectSegmentationModel>> objects)
{
    m_objects = objects;

    objectsChanged(m_objects);
}

} // namespace model
} // namespace capture

