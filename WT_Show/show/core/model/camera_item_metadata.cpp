#include "camera_item_metadata.h"

namespace capture {
namespace model {

CameraItemMetadata::CameraItemMetadata()
    : CaptureItemMetadata()
    , m_segmentation(new SegmentationCollectionModel)
    , m_documentScanMode(false)
    , m_selected(false)
    , m_colorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode::None)
{ }

bool CameraItemMetadata::selected() const
{
    return m_selected;
}

void CameraItemMetadata::setSelected(bool selected)
{
    if (m_selected != selected)
    {
        m_selected = selected;

        emit selectedChanged(m_selected);
    }
}

QSharedPointer<SegmentationCollectionModel> CameraItemMetadata::segmentation()
{
    return m_segmentation;
}

void CameraItemMetadata::setSegmentation(QSharedPointer<SegmentationCollectionModel> segmentationData)
{
    if (m_segmentation != segmentationData)
    {
        m_segmentation = segmentationData;

        emit segmentationChanged(m_segmentation);
    }
}

bool CameraItemMetadata::documentScanMode() const
{
    return m_documentScanMode;
}

void CameraItemMetadata::setDocumentScanMode(bool documentScanMode)
{
    if (m_documentScanMode != documentScanMode)
    {
        m_documentScanMode = documentScanMode;

        emit documentScanModeChanged(m_documentScanMode);
    }
}

VideoStreamSourceModel::ColorCorrectionMode CameraItemMetadata::colorCorrectionMode() const
{
    return m_colorCorrectionMode;
}

void CameraItemMetadata::setColorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode mode)
{
    if (mode != m_colorCorrectionMode)
    {
        m_colorCorrectionMode = mode;

        emit colorCorrectionModeChanged(m_colorCorrectionMode);
    }
}

} // namespace model
} // namespace capture
