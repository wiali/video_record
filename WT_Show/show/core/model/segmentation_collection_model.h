#pragma once
#ifndef SEGMENTATIONCOLLECTIONMODEL_H
#define SEGMENTATIONCOLLECTIONMODEL_H

#include <QObject>
#include <QSharedPointer>
#include <QVector>

#include "object_segmentation_model.h"

namespace capture {
namespace model {

class SegmentationCollectionModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVector<QSharedPointer<ObjectSegmentationModel>> objects READ objects WRITE setObjects NOTIFY objectsChanged)
public:
    explicit SegmentationCollectionModel(QObject *parent = 0);

    QVector<QSharedPointer<ObjectSegmentationModel>> objects() const;

signals:

    void objectsChanged(QVector<QSharedPointer<ObjectSegmentationModel>> objects);

public slots:

    void setObjects(QVector<QSharedPointer<ObjectSegmentationModel>> objects);

private:

    QVector<QSharedPointer<ObjectSegmentationModel>> m_objects;
};

} // namespace model
} // namespace capture

#endif // SEGMENTATIONCOLLECTIONMODEL_H
