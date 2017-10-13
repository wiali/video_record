#pragma once
#ifndef PLAYGROUNDITEMMODEL_H
#define PLAYGROUNDITEMMODEL_H

#include "capture_item_metadata.h"
#include "segmentation_collection_model.h"
#include "model/video_stream_source_model.h"

#include <stage_item.h>
#include <sensor_data.h>
#include <segmentation_data.h>

#include <QObject>
#include <QColor>
#include <QSharedPointer>

namespace capture {
namespace model {

class CameraItemMetadata : public CaptureItemMetadata
{
    Q_OBJECT
    Q_PROPERTY(QSharedPointer<SegmentationCollectionModel> segmentation READ segmentation WRITE setSegmentation NOTIFY segmentationChanged)
    Q_PROPERTY(bool documentScanMode READ documentScanMode WRITE setDocumentScanMode NOTIFY documentScanModeChanged)
    Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode READ colorCorrectionMode WRITE setColorCorrectionMode NOTIFY colorCorrectionModeChanged)

public:
    explicit CameraItemMetadata();

    virtual QSharedPointer<SegmentationCollectionModel> segmentation();
    QRgb hslTint() const;
    float rotationAngle() const;
    bool documentScanMode() const;
    bool selected() const;
    model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode() const;

signals:

    void segmentationChanged(QSharedPointer<SegmentationCollectionModel> segmentation);
    void documentScanModeChanged(bool documentScanMode);
    void selectedChanged(bool selected);
    void colorCorrectionModeChanged(capture::model::VideoStreamSourceModel::ColorCorrectionMode mode);

public slots:

    void setSegmentation(QSharedPointer<SegmentationCollectionModel> segmentation);
    void setDocumentScanMode(bool documentScanMode);
    void setSelected(bool selected);
    void setColorCorrectionMode(capture::model::VideoStreamSourceModel::ColorCorrectionMode mode);

private:
    QSharedPointer<SegmentationCollectionModel> m_segmentation;
    QRgb m_hslTint;
    bool m_documentScanMode;
    bool m_selected;
    model::VideoStreamSourceModel::ColorCorrectionMode m_colorCorrectionMode;
};

} // namespace model
} // namespace capture

#endif // PLAYGROUNDITEMMODEL_H
