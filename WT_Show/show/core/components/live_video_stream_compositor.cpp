#include "live_video_stream_compositor.h"

#include <QGuiApplication>
#include <QDebug>
#include <QOffscreenSurface>
#include <QUuid>

#include <global_utilities.h>

#include "common/utilities.h"

namespace capture {
namespace components {

struct LiveVideoStreamCompositor::CaptureItem {
public:
    LiveVideoStreamCompositor::VideoStreamMappings mappings;
    QMap<QString, int> sourceSkipFrameCount;
    QSize imageSize;
};

LiveVideoStreamCompositor::LiveVideoStreamCompositor(QSharedPointer<model::LiveCaptureModel> model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_videoPipeline(new video::source::VideoPipeline)
    , m_suspendWhenInking(true)
{
    connect(model.data(), &model::LiveCaptureModel::videoStreamSourcesChanged, this, &LiveVideoStreamCompositor::onVideoStreamSourcesChanged);
    onVideoStreamSourcesChanged(model->videoStreamSources());

    connect(model.data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &LiveVideoStreamCompositor::onVideoStreamStateChanged);
    parseSlotPositions();

    m_suspendWhenInking = GlobalUtilities::applicationSettings()->value("suspend_livestream_inking", false).toBool();
}

void LiveVideoStreamCompositor::onVideoStreamSourcesChanged(QVector<QSharedPointer<model::VideoStreamSourceModel>> videoStreamSources) {
    QVector<QSharedPointer<LiveVideoStreamSource>> currentSources;

    for (auto videoStreamSource : videoStreamSources)
    {
        const auto videoSource = videoStreamSource->videoSource();

        if (!m_sourceConnections.contains(videoSource)) {
            m_sourceConnections.insert(videoSource, QVector<QMetaObject::Connection>());
        }

        if (!m_sources.contains(videoSource)) {
            auto source = QSharedPointer<LiveVideoStreamSource>::create(videoStreamSource);
            m_sources.insert(videoSource, source);

            m_sourceConnections[videoSource] << connect(source.data(), &LiveVideoStreamSource::updated, this, &LiveVideoStreamCompositor::onSourceUpdated);
            m_sourceConnections[videoSource] << connect(source.data(), &LiveVideoStreamSource::streamFrozen, this, &LiveVideoStreamCompositor::streamFrozen);
        }

        currentSources << m_sources[videoStreamSource->videoSource()];
    }

    for (auto sourceType : m_sources.keys()) {
        if (!currentSources.contains(m_sources[sourceType])) {
            m_sources[sourceType]->stopFreezeDetection();

            for (auto connection : m_sourceConnections[sourceType]) {
                disconnect(connection);
            }

            m_sourceConnections[sourceType].clear();
            m_sources.remove(sourceType);
        }
    }
}

void LiveVideoStreamCompositor::restartFreezeDetectionTimers() {
    for (auto type : m_model->selectedVideoStreamSources()) {
        m_sources[type]->restartFreezeDetectionTimer();
    }
}

void LiveVideoStreamCompositor::stopFreezeDetection() {
    for (auto source : m_sources.values()) {
        source->stopFreezeDetection();
    }
}

QSharedPointer<video::source::VideoPipeline> LiveVideoStreamCompositor::videoPipeline() const { return m_videoPipeline; }

void LiveVideoStreamCompositor::onSourceUpdated(QString sourceName) {
    m_missingFirstFrameList.removeOne(sourceName);

    if (m_missingFirstFrameList.count() == 0) {
        emit updated();
    }

    {
        QMutexLocker locker(&m_captureMutex);

        if (!m_captureImageQueue.empty()) {
            auto& headItem = m_captureImageQueue.head();

            if (headItem.sourceSkipFrameCount.contains(sourceName)) {
                qDebug() << this << "Skipping capture frame" << headItem.sourceSkipFrameCount[sourceName];
                headItem.sourceSkipFrameCount[sourceName] = headItem.sourceSkipFrameCount[sourceName] - 1;

                if (headItem.sourceSkipFrameCount[sourceName] < 0) {
                    headItem.sourceSkipFrameCount.remove(sourceName);
                }
            }
        }
    }
}

void LiveVideoStreamCompositor::attachSourcePipelines() {
    for(auto pipelineSource : m_videoPipeline->sources()) {
        for(auto videoStreamSource : m_model->videoStreamSources()) {
            if (pipelineSource->name() == videoStreamSource->pipelineName()) {
                m_sources[videoStreamSource->videoSource()]->setSourcePipeline(pipelineSource);
            }
        }
    }
}

void LiveVideoStreamCompositor::detachSourcePipelines() {
    stopFreezeDetection();

    for(auto source : m_sources.values()) {
        source->setSourcePipeline(QSharedPointer<video::source::SourcePipeline>());
    }
}

void LiveVideoStreamCompositor::onVideoStreamStateChanged(model::LiveCaptureModel::VideoStreamState state) {
    if (state == model::LiveCaptureModel::VideoStreamState::Starting) {
        m_missingFirstFrameList.clear();

        for(auto type : m_model->selectedVideoStreamSources()) {
            m_missingFirstFrameList << m_model->videoStreamSource(type)->pipelineName();
        }
    }
}

void LiveVideoStreamCompositor::parseSlotPositions() {
    auto settings = GlobalUtilities::applicationSettings("composited_video");

    bool ok = false;
    // Positions need to be inverted because OpenGL has inverted Y axis
    QStringList defaultSlotPositions = { "0;0;1;1", "0.6;0.05;0.3;0.3","0.05;0.05;0.3;0.3", "0.05;0.65;0.3;0.3", "0.6;0.65;0.3;0.3" };

    for(int i = 0; i < defaultSlotPositions.length(); i++) {
        auto slotPosition = settings->value(QString("slot_position_%1").arg(i), defaultSlotPositions[i]).toString();
        auto coordinates = slotPosition.split(";", QString::SplitBehavior::SkipEmptyParts);

        if (coordinates.length() != 4) {
            qWarning() << this << "Slot" << i << "coordinates must contain 4 float numbers, got" << coordinates;
            slotPosition = defaultSlotPositions[i];
        } else {
            for (auto coordinate : coordinates) {
                coordinate.toDouble(&ok);
                if (!ok) {
                    qWarning() << this << "Slot" << i << "coordinate" << coordinate << "must be a float number";
                }
            }

            if (!ok) {
                slotPosition = defaultSlotPositions[i];
            }
        }

        coordinates = slotPosition.split(";", QString::SplitBehavior::SkipEmptyParts);
        QVector<qreal> values;
        for (auto coordinate : coordinates) {
            values << coordinate.toDouble(&ok);
        }

        m_slotPositions << QRectF(values[0], values[1], values[2], values[3]);
        qDebug() << this << "Slot" << i << "will be at position" << m_slotPositions.last();
    }
}

LiveVideoStreamCompositor::VideoStreamMappings LiveVideoStreamCompositor::videoStreamMappings(const QRect& widgetRectangle, const QTransform& primaryTransform) {
    QRect sourceRect(QPoint(), frameSize());

    // Invert Y axis because OpenGL operates in inverted Y mode
    sourceRect.setRect(sourceRect.x(), sourceRect.height() - sourceRect.y(),
                       sourceRect.width(), -sourceRect.height());

    return videoStreamMappings(widgetRectangle, sourceRect, sourceRect, primaryTransform, false, true);
}

LiveVideoStreamCompositor::VideoStreamMappings LiveVideoStreamCompositor::videoStreamMappings(
        const QRectF& widgetRectangle,
        const QRectF& primarySourceRectangle,
        const QRectF& primaryTransformRectangle,
        const QTransform& primaryTransform,
        bool invertDestinationRect,
        bool useWidgetRectangleForSecondaryStreams) {
    auto selectedVideoStreamSources = m_model->selectedVideoStreamSources();
    QHash<common::VideoSourceInfo, VideoStreamMapping> result;
    QRectF primaryDestinationRect;

    for (int i = 0; i < selectedVideoStreamSources.length(); i++) {
        const auto type = selectedVideoStreamSources.at(i);
        const auto widgetSource = m_sources[type];
        const auto slotPosition = m_slotPositions[i];
        const auto frameSize = widgetSource->frameSize();

        if (!frameSize.isEmpty()) {
            auto sourceRect = i == 0 ? primarySourceRectangle : QRectF(QPointF(), frameSize);
            QRectF destinationRect;

            if (i == 0) {
                // Keep the background rectangle information for picture in picture streams
                destinationRect = primaryTransform.mapRect(primaryTransformRectangle);

                // SPROUTSW-4738 - The Front cam/Vertical screen/Mat screen also can be zoomed in when zoom in live capture with all sources on one screen.
                if (useWidgetRectangleForSecondaryStreams) {
                    primaryDestinationRect = widgetRectangle;

                    // Make sure that boundaries for secondary streams are bound to displayed rectangle
                    if (destinationRect.top() > primaryDestinationRect.top()) {
                        primaryDestinationRect.setTop(destinationRect.top());
                    }
                    if (destinationRect.left() > primaryDestinationRect.left()) {
                        primaryDestinationRect.setLeft(destinationRect.left());
                    }
                    if (destinationRect.right() < primaryDestinationRect.right()) {
                        primaryDestinationRect.setRight(destinationRect.right());
                    }
                    if (destinationRect.bottom() < primaryDestinationRect.bottom()) {
                        primaryDestinationRect.setBottom(destinationRect.bottom());
                    }
                } else {
                    primaryDestinationRect = destinationRect;
                }

                if (invertDestinationRect) {
                    destinationRect.setRect(destinationRect.x(), widgetRectangle.height()- destinationRect.y(),
                                            destinationRect.width(), -destinationRect.height());
                }
            } else {
                // Invert Y axis because OpenGL operates in inverted Y mode
                sourceRect.setRect(sourceRect.x(), sourceRect.height() - sourceRect.y(),
                                   sourceRect.width(), -sourceRect.height());

                const auto backgroundWidth = primaryDestinationRect.width();
                const auto backgroundHeight = primaryDestinationRect.height();
                const auto absoluteSlotHeight = slotPosition.height() * backgroundHeight;

                destinationRect = QRectF(primaryDestinationRect.x() + static_cast<int>(slotPosition.x() * backgroundWidth),
                                         primaryDestinationRect.y() + static_cast<int>(slotPosition.y() * backgroundHeight),
                                         static_cast<int>(absoluteSlotHeight * frameSize.width() / frameSize.height()),
                                         static_cast<int>(absoluteSlotHeight));
            }            

            result.insert(type, { sourceRect.toRect(), destinationRect.toRect() });
        }
    }

    return result;
}

void LiveVideoStreamCompositor::requestCapture(const QSize& size, const QRectF& viewport, bool waitForNextFrame, bool keepAspectRatio) {
    CaptureItem captureItem;
    QSizeF fullFrameSize(frameSize());
    QSizeF viewportFrameSize (fullFrameSize.width() * viewport.width(),
                                    fullFrameSize.height() * viewport.height());

    viewportFrameSize.setWidth(std::min(viewportFrameSize.width(), fullFrameSize.width()));
    viewportFrameSize.setHeight(std::min(viewportFrameSize.height(), fullFrameSize.height()));

    captureItem.imageSize = size.isValid() ? size : viewportFrameSize.toSize();
    captureItem.imageSize = keepAspectRatio ? viewportFrameSize.toSize().scaled(captureItem.imageSize, Qt::KeepAspectRatio) : captureItem.imageSize;

    auto updatedViewport = viewport;
    QRectF rect(QPoint(), captureItem.imageSize) ;
    QTransform transform = common::Utilities::transformFromViewport(&updatedViewport, fullFrameSize, rect);

    QRectF videoFrameRectangle(updatedViewport.left() * fullFrameSize.width(),
                               updatedViewport.top() * fullFrameSize.height(),
                               updatedViewport.width() * fullFrameSize.width(),
                               updatedViewport.height() * fullFrameSize.height());

    QRect transformRectangle(QPoint(), videoFrameRectangle.size().toSize());
    captureItem.mappings = videoStreamMappings(rect,
                                               videoFrameRectangle.toRect(),
                                               transformRectangle,
                                               transform, true, false);

    if (waitForNextFrame) {
        for(auto type : m_model->selectedVideoStreamSources()) {
            int skipFrameCount = 1;
            if (type == common::VideoSourceInfo::DownwardFacingCamera()) {
                skipFrameCount = GlobalUtilities::applicationSettings("capture")->value("downward_facing_camera_skip_frame_count", 3).toInt();
            }

            captureItem.sourceSkipFrameCount.insert(m_model->videoStreamSource(type)->pipelineName(), skipFrameCount);
        }
    }

    qDebug() << this << "requesting capture of size" << captureItem.imageSize << "and wait for next frame" << waitForNextFrame;

    {
        QMutexLocker locker(&m_captureMutex);        
        m_captureImageQueue.enqueue(captureItem);
    }
}

void LiveVideoStreamCompositor::blitFramebuffer(QOpenGLFramebufferObject *target, VideoStreamMappings mappings) {
    QScopedPointer<QOpenGLFramebufferObject> capturedFrameBuffer;
    VideoStreamMappings capturedFrameMappings;
    CaptureItem captureImageInfo;
    bool isValidCapture = false;

    {
        QMutexLocker locker(&m_captureMutex);

        if (!m_captureImageQueue.empty()) {
            if (m_captureImageQueue.head().sourceSkipFrameCount.count() == 0) {
                captureImageInfo = m_captureImageQueue.dequeue();
                isValidCapture = true;
            }
        }
    }

    if (isValidCapture) {
        qDebug() << this << "Capturing image ...";

        capturedFrameMappings = captureImageInfo.mappings;
        capturedFrameBuffer.reset(new QOpenGLFramebufferObject(captureImageInfo.imageSize));
    }

    for (auto type : m_model->selectedVideoStreamSources()) {
        const auto source = m_sources[type];
        const auto mapping = mappings[type];

        // not update the framebuffer if drawing the ink.
        if(!m_suspendWhenInking || !m_model->inking() || capturedFrameBuffer)
            source->updateFrameBuffer();

        // Blit to target framebuffer ...
        QOpenGLFramebufferObject::blitFramebuffer(target, mapping.destination, source->frameBuffer().data(), mapping.source,
                                                  // SPROUTSW-4416 - Using linear sampling instead of default nearest
                                                  GL_COLOR_BUFFER_BIT, GL_LINEAR);

        if (capturedFrameBuffer) {
            const auto capturedFrameMapping = capturedFrameMappings[type];

            QOpenGLFramebufferObject::blitFramebuffer(capturedFrameBuffer.data(), capturedFrameMapping.destination,
                                                      source->frameBuffer().data(), capturedFrameMapping.source,
                                                      // SPROUTSW-4416 - Using linear sampling instead of default nearest
                                                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }
    }

    if (capturedFrameBuffer) {
        auto settings = GlobalUtilities::applicationSettings("composited_video");
        const auto image = capturedFrameBuffer->toImage();
        const auto storagePath = settings->value("captured_images_folder", QString()).toString();

        if (!storagePath.isNull()) {
            image.save(QString("%1/%2.jpg").arg(storagePath).arg(QUuid::createUuid().toString()));
        }

        emit captureReady(image);
    }
}

QSize LiveVideoStreamCompositor::frameSize() const {
    QSize result;
    if (m_model->selectedVideoStreamSources().count() > 0) {
        const auto type = m_model->selectedVideoStreamSources().first();
        result = m_sources[type]->frameSize();
    }

    return result;
}

} // namespace components
} // namespace capture
