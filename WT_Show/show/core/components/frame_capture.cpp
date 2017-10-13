#include "frame_capture.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonDocument>
#include <QtConcurrentRun>
#include <QFutureSynchronizer>
#include <QMutex>

#include <calibration_data.h>
#include <global_utilities.h>

#include "event/change_mat_mode_event.h"
#include "event/segment_object_event.h"
#include "event/capture_frame_event.h"
#include "event/prepare_frame_capture_event.h"
#include "common/utilities.h"
#include "common/final_act.h"
#include "common/measured_block.h"
#include "common/ink_manger.h"

namespace capture {
namespace components {

struct FrameCapture::CaptureData {
public:
    bool prepareCapture;
    bool captureWithFlash;
    bool captureNextFrame;
    QVector<common::VideoSourceInfo> videoSources;
    QRectF viewport;
    QSharedPointer<InkData> inkData;
    model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode;
};

struct FrameCapture::CaptureResult {
public:
    QVector<sensordata::SensorData> sensorData;
    QImage liveCaptureImage;
};

FrameCapture::FrameCapture(QSharedPointer<components::LiveVideoStreamCompositor> compositor,
                           QSharedPointer<model::ApplicationStateModel> model, QObject *parent)
    : QObject(parent)
    , m_compositor(compositor)
    , m_imageEnhancement(new image_enhancement::ImageEnhancement)
    , m_model(model)
    , m_threadPool(new QThreadPool)
    , m_depthCameraThread(new QThread){
    setAutoDelete(false);

    connect(m_model.data(), &model::ApplicationStateModel::matModeStateChanged, this, &FrameCapture::onMatModeStateChanged);
    connect(m_compositor.data(), &LiveVideoStreamCompositor::captureReady, this, &FrameCapture::onCaptureReady);
    connect(m_model->liveCapture().data(), &model::LiveCaptureModel::depthCameraIndexChanged, this, &FrameCapture::updateDepthStreams);

    if (GlobalUtilities::applicationSettings("capture")->value("run_depth_camera_in_live_capture", false).toBool()) {
        connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this, &FrameCapture::updateDepthStreams);
        connect(m_model->liveCapture().data(), &model::LiveCaptureModel::selectedVideoStreamSourcesChanged, this, &FrameCapture::updateDepthStreams);
        updateDepthStreams();
    }

    QCoreApplication::instance()->installEventFilter(this);

    m_depthCameraThread->start();
}

FrameCapture::~FrameCapture() {
    abort();
}

void FrameCapture::updateDepthStreams() {
    QMutexLocker locker(&m_depthCameraMutex);
    MEASURED_BLOCK

    if (m_model->liveCapture()->depthCameraIndex() < 0) {
        m_depthCamera.reset();
    } else if (m_depthCamera.isNull() || m_depthCamera->info().index != m_model->liveCapture()->depthCameraIndex()) {
        m_depthCamera.reset(new proapi::depth_camera::DepthCamera(m_model->liveCapture()->depthCameraIndex()));

        // SPROUTSW-4574 - Make sure that depth camera receives suspend/resume signals
        m_depthCamera->moveToThread(m_depthCameraThread.data());

        connect(m_depthCamera.data(), &proapi::depth_camera::DepthCamera::enabledStreamsChanged, this, &FrameCapture::updateDepthStreams);
    }       

    if (m_depthCamera) {
        QStringList requiredStreams { "ir", "depth" };

        if (m_model->mode() == model::ApplicationStateModel::Mode::LiveCapture &&
            supportsOrbbecCapture(m_model->liveCapture()->selectedVideoStreamSources())) {

            for(const auto enabledStream : m_depthCamera->currentlyEnabledStreams().streams) {
                requiredStreams.removeOne(enabledStream);
            }

            if (requiredStreams.length() > 0) {
                qInfo() << this << "Enabling depth camera streams" << requiredStreams;
                m_depthCamera->enableStreams(requiredStreams);
            }
        } else {            
            QStringList streamsToDisable;

            for(const auto enabledStream : m_depthCamera->currentlyEnabledStreams().streams) {
                if (requiredStreams.contains(enabledStream)) {
                    streamsToDisable << enabledStream;
                }
            }

            if (streamsToDisable.length() > 0) {
                qInfo() << this << "Disabling depth camera streams" << streamsToDisable;
                m_depthCamera->disableStreams(streamsToDisable);
            }
        }
    }
}

void FrameCapture::abort() {
    if (m_depthCameraThread->isRunning()) {
        m_depthCameraThread->quit();
        m_depthCameraThread->wait();
    }
}

void FrameCapture::onCaptureReady(QImage image) {
    m_liveCaptureImage = image;
    m_liveImageCaptureEvent.wakeAll();
}

void FrameCapture::onMatModeStateChanged(model::ApplicationStateModel::MatModeState matModeState) {
    if (matModeState == model::ApplicationStateModel::MatModeState::Flash) {
        m_waitForFlashEvent.wakeAll();
    }
}

bool FrameCapture::eventFilter(QObject *obj, QEvent *event) {
    bool processed = false;

    if (event->type() == event::CaptureFrameEvent::type()) {
        if (auto captureFrameEvent = static_cast<event::CaptureFrameEvent*>(event)) {
            QMutexLocker locker(&m_mutex);
            m_captureQueue.enqueue(CaptureData {
                                       false,
                                       captureFrameEvent->captureWithFlash(),
                                       captureFrameEvent->captureNextFrame(),
                                       captureFrameEvent->videoSources(),
                                       captureFrameEvent->viewport(),
                                       captureFrameEvent->inkData(),
                                       captureFrameEvent->colorCorrectionMode()
                                   });

            m_threadPool->start(this);
            processed = true;
        }
    }

    if (event->type() == event::PrepareFrameCaptureEvent::type()) {
        if (auto prepareFrameCaptureEvent = static_cast<event::PrepareFrameCaptureEvent*>(event)) {
            QMutexLocker locker(&m_mutex);
            m_captureQueue.enqueue(CaptureData {
                                       true,
                                       prepareFrameCaptureEvent->captureWithFlash(),
                                       false,
                                       prepareFrameCaptureEvent->videoSources(),
                                       QRectF(),
                                       QSharedPointer<InkData>()
                                   });

            m_threadPool->start(this);
            processed = true;
        }
    }

    return processed ? processed : QObject::eventFilter(obj, event);
}

void FrameCapture::run() {
    bool haveItemInQueue = false;
    CaptureData captureData;

    {
        QMutexLocker locker(&m_mutex);
        haveItemInQueue = !m_captureQueue.isEmpty();

        if (haveItemInQueue) {
            captureData = m_captureQueue.dequeue();
        }
    }

    if (haveItemInQueue) {
        try {
            if (captureData.prepareCapture) {
                prepareCapture(captureData.captureWithFlash, captureData.videoSources);
            } else  {
                performCapture(captureData.captureWithFlash, captureData.captureNextFrame, captureData.colorCorrectionMode,
                               captureData.videoSources, captureData.viewport, captureData.inkData);
            }
        }
        catch(...) {
            emit captureFailed();
        }
    }
}

void FrameCapture::prepareCapture(bool captureWithFlash, QVector<common::VideoSourceInfo> videoSources) {
    QMutexLocker locker(&m_captureMutex);
    MEASURED_BLOCK

    auto liveCapture = m_model->liveCapture();
    liveCapture->setCaptureState(model::LiveCaptureModel::CaptureState::PreparingCapture);

    static QHash<model::ApplicationStateModel::MatModeState, model::LiveCaptureModel::PreCaptureMode> preCaptureModeTranslationTable {
         { model::ApplicationStateModel::MatModeState::Desktop, model::LiveCaptureModel::Desktop },
    { model::ApplicationStateModel::MatModeState::LampOff, model::LiveCaptureModel::LampOff },
    { model::ApplicationStateModel::MatModeState::LampOn, model::LiveCaptureModel::LampOn }
    };

    liveCapture->setPreCaptureMode(preCaptureModeTranslationTable[m_model->matModeState()]);

    if (captureWithFlash) {
        auto captureModeEvent = new event::ChangeMatModeEvent(event::ChangeMatModeEvent::Flash);
        captureModeEvent->dispatch();

        qDebug() << this << "Waiting for flash ...";
    }

    if (supportsOrbbecCapture(videoSources)) {
        auto orbbecMeasuredBlock = common::measure_block(__FUNCTION__ "::orbbec"); Q_UNUSED(orbbecMeasuredBlock);

        updateDepthStreams();

        m_depthCamera->setIrFloodOn(true);
        m_depthCamera->setLaserOn(false);
    }

    if (captureWithFlash && m_model->matModeState() != model::ApplicationStateModel::MatModeState::Flash) {
        QMutex flashMutex;
        QMutexLocker flashLocker(&flashMutex);

        m_waitForFlashEvent.wait(&flashMutex);
        qDebug() << this << "Flash mode on";
    }

    liveCapture->setCaptureState(model::LiveCaptureModel::CaptureState::CapturePrepared);
}

void FrameCapture::performCapture(bool captureWithFlash, bool captureNextFrame,
                                  model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode,
                                  QVector<common::VideoSourceInfo> videoSources,
                                  const QRectF& viewport, QSharedPointer<InkData> inkData) {
    auto liveCapture = m_model->liveCapture();
    CaptureResult captureResult;

    auto finalizeCapture = common::finally([liveCapture, videoSources, this] {
        updateDepthStreams();

        liveCapture->setCaptureState(model::LiveCaptureModel::CaptureState::NotCapturing);
    });

    {
        MEASURED_BLOCK

        {
            qInfo() << this << "Waiting for preparation to finish ...";
            QMutexLocker locker(&m_captureMutex);
            liveCapture->setCaptureState(model::LiveCaptureModel::CaptureState::Capturing);

            captureResult = capture(captureWithFlash, captureNextFrame, viewport, videoSources);

            liveCapture->setCaptureState(model::LiveCaptureModel::CaptureState::FinalizingCapture);
        }

        auto stageProject = createStageProject(videoSources, viewport, inkData, colorCorrectionMode, captureResult);

        if (m_model->mainWindowLocation() == model::ApplicationStateModel::MainWindowLocation::None &&
                !m_model->singleScreenMode()) {
            // Force the mode to avoid problems with projector
            m_model->setMatModeState(model::ApplicationStateModel::MatModeState::Reprojection);

            auto captureModeEvent = new event::ChangeMatModeEvent(event::ChangeMatModeEvent::MatMode::Reprojection);
            captureModeEvent->dispatch();
        }

        m_model->setMode(model::ApplicationStateModel::Mode::Preview);

        // Update image to match what was actually captured

        if (videoSources.count() > 1) {
            QVector<sensordata::SensorData> hiresRgbSensorData;
            hiresRgbSensorData << captureResult.sensorData.first();

            if (GlobalUtilities::applicationSettings("capture")->value("perform_image_enhancement", false).toBool()) {
                auto image = m_imageEnhancement->convertCapture(hiresRgbSensorData);
                stageProject->items().first()->setImage(image);
                // Force thumbnail update
                stageProject->setThumbnail(image);
            }
        }

        liveCapture->setCaptureState(model::LiveCaptureModel::CaptureState::NotCapturing);
    }
}

QVector<sensordata::SensorData> FrameCapture::captureCameraData(QVector<common::VideoSourceInfo> videoSources, bool captureWithFlash) {
    QStringList stillCaptureSources;
    try {
        MEASURED_BLOCK
        QStringList stillCaptureSources;

        if (captureWithFlash && videoSources.contains(common::VideoSourceInfo::SproutCamera())) {
            stillCaptureSources << m_model->liveCapture()->videoStreamSource(common::VideoSourceInfo::SproutCamera())->pipelineName();
        }

        auto result = m_compositor->videoPipeline()->capture(stillCaptureSources);

        return result;
    }
    catch(std::exception &ex) {
        qCritical() << this << "Failed to perform video frame capture, reason" << ex.what();
        throw QException();
    }
    catch(std::string &what) {
        qCritical() << this << "Failed to perform video frame capture, reason" << QString::fromStdString(what);
        throw QException();
    }
    catch(...) {
        qCritical() << this << "Failed to perform video frame capture with unknown exception";
        throw QException();
    }
}

QVector<sensordata::SensorData> FrameCapture::captureOrbbecData() {
    QVector<sensordata::SensorData> result;

    static auto QImageCleanup = [](void *info) { delete[] info; };

    try {
        MEASURED_BLOCK

        QMap<QString, proapi::depth_camera::Frame> captureResult;

        const auto irFrame = m_depthCamera->grabFrames( { "ir" }, false, 0);

        if (irFrame.count() > 0) {
            captureResult.insert(irFrame.firstKey(), irFrame.first());
        } else {
            const auto error = m_depthCamera->error();
            qCritical() << "Orbbec IR capture failed with error" << error.code << error.message << error.data;
            throw std::exception(m_depthCamera->error().message.toStdString().c_str());
        }

        m_depthCamera->setIrFloodOn(false);
        m_depthCamera->enableStreams( { "depth" } );
        m_depthCamera->setLaserOn(true);

        const auto depthFrame = m_depthCamera->grabFrames( { "depth" }, false, 0);
        if (depthFrame.count() == 0) {
            const auto error = m_depthCamera->error();
            qCritical() << "Orbbec depth capture failed with error" << error.code << error.message << error.data;
            throw std::exception(m_depthCamera->error().message.toStdString().c_str());
        } else {
            captureResult.insert(depthFrame.firstKey(), depthFrame.first());

            for(auto frame : captureResult.values()) {
                auto buffer = new uchar[frame.byteCount()];
                ::memcpy(buffer, frame.data(), frame.byteCount());

                // Since we are not capturing RGB we can set format QImage::Format_RGB16 for both frames
                QImage image(buffer, frame.width(), frame.height(), frame.byteCount() / frame.height(), QImage::Format_RGB16, QImageCleanup);

                // We will copy calibration data from hires camera frame
                result << sensordata::SensorData(image.mirrored(true, false), "orbbec",
                                                 frame.stream(), QDateTime::currentDateTime(), calibration::CalibrationData(), QJsonObject());
            }
        }
    }
    catch(std::exception& ex) {
        qCritical() << "Exception occured during Orbbec capture" << typeid(ex).name() << ":" << ex.what();
    }

    return result;
}

QImage FrameCapture::captureLiveFrame(bool captureNextFrame, const QRectF& viewport) {
    QImage result;

    try {
        MEASURED_BLOCK

        QMutex mutex;
        QMutexLocker captureLocker(&mutex);

        m_liveCaptureImage = QImage();
        m_compositor->requestCapture(QSize(), viewport, captureNextFrame);
        m_liveImageCaptureEvent.wait(&mutex);

        result = m_liveCaptureImage;
    }
    catch(std::exception& ex) {
        qCritical() << "Exception occured during Live frame" << typeid(ex).name() << ":" << ex.what();
    }

    return result;
}


QSharedPointer<StageProject> FrameCapture::createStageProject(QVector<common::VideoSourceInfo> videoSources,
                                                              QRectF viewport, QSharedPointer<InkData> inkData,
                                                              model::VideoStreamSourceModel::ColorCorrectionMode colorCorrectionMode,
                                                              CaptureResult captureResult) {
    if (captureResult.liveCaptureImage.isNull()) {
        throw std::exception("Could not capture image from input thread");
    }

    auto metadata = QSharedPointer<model::CameraItemMetadata>::create();
    metadata->setSensorData(&captureResult.sensorData);
    metadata->setColorCorrectionMode(colorCorrectionMode);

    // In case that we are capturing more than one source the viewport will become zoomed in part of the image
    metadata->setCapturedViewport(videoSources.count() == 1 ? viewport : QRect(0, 0, 1, 1));

    auto stageItem = QSharedPointer<StageItem>::create(captureResult.liveCaptureImage, metadata);

    QSharedPointer<StageProject> project = QSharedPointer<StageProject>::create();
    project->setThumbnail(captureResult.liveCaptureImage);

    auto now = QDateTime::currentDateTime();

    project->setName(QString(tr("HP_IMG_%1_%2")).arg(now.toString("yyyyMMdd")).arg(now.toString("HHmmss")));
    project->addItem(stageItem);
    project->inkData()->clone(*inkData.data());

    // set ink canvas size (for the front facing camera it's going to be 1920x1080 vs 1920x1280 for the downward facing)
    if (!m_model->singleScreenMode())
    {
        calcInkPosition(viewport, captureResult.liveCaptureImage.size(), stageItem, videoSources.count() > 1);

        if (auto matScreen = GlobalUtilities::findScreen(GlobalUtilities::MatScreen)) {
            QSize inkCanvasSize = stageItem->image().size().scaled(matScreen->size(), Qt::KeepAspectRatio);
            project->inkData()->setCanvasSize(inkCanvasSize);
        }
    }

    if (supportsOrbbecCapture(videoSources) && videoSources.count() == 1) {
        // Start segmenting the object right away only if we have all the data we need
        auto segmentationEvent = new event::SegmentObjectEvent(metadata);
        segmentationEvent->dispatch();
    }
    else
    {
        metadata->setSegmentationState(CaptureItemMetadata::SegmentationState::Disabled);
    }

    m_model->projects()->add(project);

    return project;
}

bool FrameCapture::supportsOrbbecCapture(QVector<common::VideoSourceInfo> videoSources) {
    return videoSources.contains(common::VideoSourceInfo::DownwardFacingCamera()) ||
            videoSources.contains(common::VideoSourceInfo::SproutCamera());
}

FrameCapture::CaptureResult FrameCapture::capture(bool captureWithFlash, bool captureNextFrame,
                                                  const QRectF& viewport, QVector<common::VideoSourceInfo> videoSources) {
    QVector<sensordata::SensorData> sensorData;
    qInfo() << "Capturing ...";

    bool requiresDepthFrame = supportsOrbbecCapture(videoSources);

    QFuture<QVector<sensordata::SensorData>> cameraData, orbbecData;

    // In case that we are capturing more than one source the viewport will become zoomed in part of the image
    auto liveCaptureImage = QtConcurrent::run(this, &FrameCapture::captureLiveFrame, captureNextFrame,
                                              videoSources.count() == 1 ? QRect(0, 0, 1, 1) : viewport);

    if (videoSources.count() == 1) {
        cameraData = QtConcurrent::run(this, &FrameCapture::captureCameraData, videoSources, captureWithFlash);

        if (requiresDepthFrame) {
            orbbecData = QtConcurrent::run(this, &FrameCapture::captureOrbbecData);
        }

        cameraData.waitForFinished();

        sensorData = cameraData.result();

        if (requiresDepthFrame) {
            // Copy calibration data for depth/ir from hires frame
            for(auto data : orbbecData.result()) {
                data.calibrationData = sensorData.first().calibrationData;
                sensorData << data;
            }
        }
    }

    liveCaptureImage.waitForFinished();

    if (videoSources.count() > 1) {
        sensordata::SensorData compositedData;
        compositedData.image = liveCaptureImage.result();
        compositedData.captureDevice = "composited_stream";
        compositedData.type = "hiresrgb";
        compositedData.timeStamp = QDateTime::currentDateTime();

        sensorData << compositedData;
    }

    return CaptureResult { sensorData, liveCaptureImage.result() };
}

void FrameCapture::calcInkPosition(const QRectF& viewport, const QSize& captureImageSize,
    QSharedPointer<StageItem> stageItem, bool multiSources)
{
    auto matScreen = GlobalUtilities::findScreen(GlobalUtilities::MatScreen);
    QSize matSize = matScreen->size();
    QRectF matGeometry(0, 0, matSize.width(), matSize.height());
    auto ink = m_model->liveCapture()->inkData();

    if (!multiSources)
    {
        for (int i = 0; i < ink->strokeCount(); i++)
        {
            QPolygonF corners(matGeometry);
            corners.removeLast();
            common::InkManger::addStroke(ink->stroke(i), stageItem, corners);
        }
        return;
    }

    QRectF srcRect(matSize.width() * viewport.left(), matSize.height() * viewport.top(),
        matSize.width() * viewport.width(), matSize.height() * viewport.height());

    // Round it up to nearest pixel since no zoom not mean viewport(0,0,1,1).
    srcRect.setX(std::max(std::round(srcRect.x()), 0.0));
    srcRect.setY(std::max(std::round(srcRect.y()), 0.0));
    srcRect.setWidth(std::min(std::round(srcRect.width()), static_cast<qreal>(matSize.width())));
    srcRect.setHeight(std::min(std::round(srcRect.height()), static_cast<qreal>(matSize.height())));

    QPolygonF src(srcRect);
    src.removeLast();

    QSize imageOnMatSize = captureImageSize.scaled(matScreen->size(), Qt::KeepAspectRatio);
    QRectF dstRect(QRectF(0, 0, imageOnMatSize.width(), imageOnMatSize.height()));
    QPolygonF dst(dstRect);
    dst.removeLast();

    QTransform matrix;
    if (!QTransform::quadToQuad(src, dst, matrix))
    {
        QString errorMsg = "Failed to build transform";
        qCritical() << this << errorMsg;
        throw "Failed to build transform";
    }

    for (int i = 0; i < ink->strokeCount(); i++)
    {
        QSharedPointer<InkStroke> stroke = ink->stroke(i);
        for (int j = 0; j < stroke->pointCount(); j++)
        {
            InkPoint point = stroke->point(j);
            point.point = matrix.map(point.point);
            point.size *= matrix.m22();
            stroke->setPoint(j, point);
        }

        common::InkManger::addStroke(stroke, stageItem, dst);
    }
}

} // namespace components
} // namespace capture
