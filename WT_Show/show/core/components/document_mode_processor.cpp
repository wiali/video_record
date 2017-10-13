#include "document_mode_processor.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QtConcurrentRun>
#include <QDirIterator>
#include <QDebug>
#include <QJsonDocument>
#include <QUrl>
#include <QtMath>

#include <segmentation.h>
#include <sensor_data.h>
#include <global_utilities.h>

#include "event/segment_object_event.h"
#include "common/measured_block.h"

namespace capture {
namespace components {

static QHash<QString, model::ObjectSegmentationModel::Type> StringToSegmentationType {
    { "FLAT_RECTANGLE", model::ObjectSegmentationModel::Type::FlatRectangle },
    { "FLAT_NON_RECTANGLE", model::ObjectSegmentationModel::Type::FlatNonRectangle },
    { "THREE_DIM_OBJECT", model::ObjectSegmentationModel::Type::ThreeDObject }
};

DocumentModeProcessor::DocumentModeProcessor(QObject *parent)
    : QObject(parent)
    , m_threadPool(new QThreadPool) {
    setAutoDelete(false);
    QCoreApplication::instance()->installEventFilter(this);
    m_threadPool->start(this);
}

bool DocumentModeProcessor::eventFilter(QObject *obj, QEvent *event) {
    bool processed = false;

    if (event->type() == event::SegmentObjectEvent::type()) {
        if (auto segmentObjectEvent = static_cast<event::SegmentObjectEvent*>(event)) {
            {
                QMutexLocker locker(&m_mutex);
                m_segmentationQueue.enqueue(segmentObjectEvent->model());
            }

            tryProcessNextModel();
            processed = true;
        }
    }

    return processed ? processed : QObject::eventFilter(obj, event);
}

void DocumentModeProcessor::tryProcessNextModel() {
    QMutexLocker locker(&m_mutex);

    if (!m_segmentationQueue.isEmpty()) {
        m_threadPool->start(this);
    }
}

void DocumentModeProcessor::run() {
    QSharedPointer<model::CameraItemMetadata> model;

    {
        QMutexLocker locker(&m_mutex);

        if (!m_segmentationQueue.isEmpty()) {
            model = m_segmentationQueue.dequeue();
        }
    }

    if (model && model->segmentationState() == CaptureItemMetadata::SegmentationState::NotStarted) {
        auto settings = GlobalUtilities::applicationSettings("segmentation");

        // Delay start of the segmentation to get better user experience from switching to post capture
        QThread::msleep(settings->value("start_delay_ms", 150).toInt());
        MEASURED_BLOCK;

        qDebug() << this << "starting segmentation";

        // While the segmentation package it being updated to stop propagating an exception, and while
        // the texture/memory issue is fixed with a bandaid/workaround, this Try/Catch block allow us to
        // properly set the SegmentationState to Failed and avoid a an error in the QT Concurrany for now.
        try {
            auto doAlphaMatting = settings->value("alpha_matting_enabled", true).toBool();

            // For MVP1 we don't need segmentation
            auto segmentationEnabled = settings->value("segmentation_enabled", true).toBool();
            QVector<segmentation::SegmentationData> segmentationData;
            QVector<QImage> segmentedObjectImage;

            if (segmentationEnabled) {
                model->setSegmentationState(CaptureItemMetadata::SegmentationState::Segmenting);
                auto captureResults = model->sensorData();

                segmentation::Segmentation seg;
                segmentation::SegmentationOptions options;
                options.alphaMattingEnabled = doAlphaMatting;

                switch(model->colorCorrectionMode()) {
                case model::VideoStreamSourceModel::ColorCorrectionMode::None:
                    options.colorCorrection = segmentation::SegmentationOptions::ColorCorrection::None;
                    break;
                case model::VideoStreamSourceModel::ColorCorrectionMode::LampOff:
                    options.colorCorrection = segmentation::SegmentationOptions::ColorCorrection::LampOff;
                    break;
                case model::VideoStreamSourceModel::ColorCorrectionMode::LampOn:
                    options.colorCorrection = segmentation::SegmentationOptions::ColorCorrection::LampOn;
                    break;
                }

                qInfo() << this << "Using segmentation with option" << options.colorCorrection << options.alphaMattingEnabled;
                seg.processImage(captureResults.data(), &segmentationData, &segmentedObjectImage, options);
            } else {
                model->setSegmentationState(CaptureItemMetadata::SegmentationState::Disabled);
            }

            if (segmentationEnabled) {
                model->setSegmentationData(&segmentationData);

                QVector<QSharedPointer<model::ObjectSegmentationModel>> objects;

                for(int i=0; i < segmentedObjectImage.count();i++) {
                    auto metadata = segmentationData[i].geometry.metadata;

                    QRect originalRect (metadata["x"].toInt(), metadata["y"].toInt(), metadata["width"].toInt(), metadata["height"].toInt());
                    double skewAngle = metadata["skew"].toDouble();
                    objects << QSharedPointer<model::ObjectSegmentationModel>::create(originalRect,
                                                                                      skewAngle,
                                                                                      StringToSegmentationType[metadata["objectType"].toString()],
                            segmentationData[i].geometry);

                    if (objects.last()->type() == model::ObjectSegmentationModel::Type::FlatRectangle) {
                        qDebug() << this << "Found document with skew angle" << objects.last()->skewAngle();
                    }
                }

                model->segmentation()->setObjects(objects);
                model->setSegmentationState(CaptureItemMetadata::SegmentationState::Finished);
            } else {
                // ToDo: Expose segmentation error if needed

                model->setSegmentationState(CaptureItemMetadata::SegmentationState::Failed);
            }
        }
        catch(...) {
            // I have thrown exceptions while testing to validate the logic in  Capture, and also the logic
            // in the Document Mode and Background Removal, work fine when SegmentationState::Failed... everything behaved as expected \o/
            model->setSegmentationState(CaptureItemMetadata::SegmentationState::Failed);
        }
    }
}

} // namespace components
} // namespace capture
