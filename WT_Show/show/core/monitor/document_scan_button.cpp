#include "document_scan_button.h"

#include <QSvgRenderer>
#include <QQuaternion>
#include <QPainter>

#include "model/camera_item_metadata.h"

namespace capture {
namespace monitor {

DocumentScanButton::DocumentScanButton(QWidget *parent)
    : RightMenuButton(parent)
{
    setIconName("icon-docmode");
    setText(tr("Doc Mode"));

    connect(this, &RightMenuButton::clicked, this, &DocumentScanButton::onDocModeButtonClicked);
}

void DocumentScanButton::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;

    connect(m_model.data(), &model::ApplicationStateModel::selectedProjectChanged, this, &DocumentScanButton::onSelectedProjectChanged);
    connect(m_model->projectsExport().data(), &model::ProjectsExportModel::stateChanged, this, &DocumentScanButton::onDocumentScanModeChanged);
    onSelectedProjectChanged();
}

void DocumentScanButton::onSelectedProjectChanged()
{
    bool disable = true;

    // Disconnect last handlers
    if (m_modelStatusConnection)
    {
        disconnect(m_modelStatusConnection);
    }

    if (m_documentModeConnection)
    {
        disconnect(m_documentModeConnection);
    }

    auto project = m_model->selectedProject();

    if (project)
    {
        if (project->items().count() > 0)
        {
            auto metadata = project->items().first()->metadata();
            auto cameraMetadata = metadata.dynamicCast<model::CameraItemMetadata>();

            if (cameraMetadata)
            {
                m_modelStatusConnection = connect(cameraMetadata.data(), &model::CameraItemMetadata::segmentationStateChanged, this, &DocumentScanButton::onModelSegmentationStatusChanged);
                onModelSegmentationStatusChanged(cameraMetadata->segmentationState());

                m_documentModeConnection = connect(cameraMetadata.data(), &model::CameraItemMetadata::documentScanModeChanged, this, &DocumentScanButton::onDocumentScanModeChanged);
                onDocumentScanModeChanged();

                disable = false;
            }
        }
    }

    if (disable)
    {
        setEnabled(false);
    }
}

void DocumentScanButton::onDocumentScanModeChanged()
{
    bool isChecked = false;

    auto project = m_model->selectedProject();

    if (project)
    {
        auto item = project->items().first();
        auto metaData = item->metadata().dynamicCast<model::CameraItemMetadata>();

        isChecked = metaData->documentScanMode();

        foreach (auto segmentedObject, metaData->segmentation()->objects())
        {
            if (segmentedObject->type() == model::ObjectSegmentationModel::Type::FlatRectangle)
            {
                float angle = 0;
                QImage mask = metaData->objectSegAlphaMask();

                mask.fill(Qt::white);

                if (isChecked)
                {
                    QPainter painter(&mask);
                    auto svg = segmentedObject->geometry().svg;

                    QSvgRenderer svgRenderer(svg.toLocal8Bit());

                    svgRenderer.render(&painter);
                    angle = segmentedObject->skewAngle();

                    mask.invertPixels();
                }

                metaData->setObjectSegAlphaMask(mask);
                metaData->setRotation(QQuaternion::fromAxisAndAngle(0, 0, 1, angle).toEulerAngles());

                break;
            }
        }
    }

    setChecked(isChecked);
}


void DocumentScanButton::onModelSegmentationStatusChanged(CaptureItemMetadata::SegmentationState newState)
{
    bool isEnabled = false;
    auto project = m_model->selectedProject();

    if (project)
    {
        auto item = project->items().first();
        auto metaData = item->metadata().dynamicCast<model::CameraItemMetadata>();
        auto segmentation = metaData->segmentation();

        if (newState == CaptureItemMetadata::SegmentationState::Finished)
        {
            qInfo() << this << "Segmentation finished with" << segmentation->objects().count() << "objects:";

            foreach (auto segmentedObject, segmentation->objects())
            {
                qInfo() << this << "Segmentation object type" << segmentedObject->type() << ", skewAngle:" << segmentedObject->skewAngle();

                if (segmentedObject->type() == model::ObjectSegmentationModel::Type::FlatRectangle)
                {
                    isEnabled = true;
                    break;
                }
            }
        }
    }

    setEnabled(isEnabled);
}

void DocumentScanButton::onDocModeButtonClicked()
{
    auto project = m_model->selectedProject();

    if (project)
    {
        auto item = project->items().first();
        auto metaData = item->metadata().dynamicCast<model::CameraItemMetadata>();

        metaData->setDocumentScanMode(!metaData->documentScanMode());
    }
}

} // namespace monitor
} // namespace capture

