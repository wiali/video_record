#include "video_source_manager.h"

#include <QCamera>

#include <global_utilities.h>

#include "common/utilities.h"
#include "event/start_video_streaming_event.h"
#include "event/stop_video_streaming_event.h"

#include "camera/camera.h"

namespace capture {
namespace components {

VideoSourceManager::VideoSourceManager(QSharedPointer<model::ApplicationStateModel> model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_system(new proapi::hal::System)
    , m_eventHandler(new user_event_handler::EventHandler) {
    connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this, &VideoSourceManager::onApplicationModeChanged);
    connect(m_model.data(), &model::ApplicationStateModel::picInPicModeChanged, this, &VideoSourceManager::onPicInPicModeChanged);
    connect(m_eventHandler.data(), &user_event_handler::EventHandler::displayCountChanged, this, &VideoSourceManager::onDisplayCountChanged);
    onDisplayCountChanged(m_eventHandler->displayCount());

    auto liveCaptureModel = m_model->liveCapture();

    connect(liveCaptureModel.data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &VideoSourceManager::onCameraStateChanged);
    connect(liveCaptureModel.data(), &model::LiveCaptureModel::autoFixChanged, this, &VideoSourceManager::updateColorCorrection);

    connect(m_model.data(), &model::ApplicationStateModel::matModeStateChanged, this, &VideoSourceManager::onMatModeStateChanged);
    onMatModeStateChanged(m_model->matModeState());

    connect(liveCaptureModel.data(), &model::LiveCaptureModel::selectedVideoStreamSourcesChanged, this,
            &VideoSourceManager::onSelectedVideoStreamSourcesChanged);

    // Calls also updateColorCorrection()
    onCameraStateChanged();    

    auto settings = GlobalUtilities::applicationSettings("camera_manager");

    findAttachedCameraDevices();

    if (m_model->singleScreenMode()) {        
        findAttachedWebcameras();

        connect(m_system.data(), &proapi::hal::System::deviceConnected, this, &VideoSourceManager::onDeviceConnected);
        connect(m_system.data(), &proapi::hal::System::deviceDisconnected, this, &VideoSourceManager::onDeviceDisconnected);

        // Temporary until SOHAL will start returning also webcams
        connect(m_eventHandler.data(), &user_event_handler::EventHandler::usbArrival, this, &VideoSourceManager::onUsbDeviceConnectionChanged);
        connect(m_eventHandler.data(), &user_event_handler::EventHandler::usbRemove, this, &VideoSourceManager::onUsbDeviceConnectionChanged);

        onDeviceConnectionChanged();

        connect(liveCaptureModel.data(), &model::LiveCaptureModel::keystoneCorrectionModeChanged, this, &VideoSourceManager::onKeystoneCorrectionModeChanged);
    } else {
        auto videoStreamSources = m_model->liveCapture()->videoStreamSources();

        videoStreamSources << QSharedPointer<model::VideoStreamSourceModel>::create(common::VideoSourceInfo::DownwardFacingCamera());
        videoStreamSources << QSharedPointer<model::VideoStreamSourceModel>::create(common::VideoSourceInfo::ForwardFacingCamera());

        m_model->liveCapture()->setVideoStreamSources(videoStreamSources);        

        // Find out which camera is Sprout camera
        for(auto device : m_sohalDevices) {
            if (device.name == "hirescamera" &&
                device.vendor_id == settings->value("sprout_hires_camera_vid", 0x05A9).toInt() &&
                device.product_id == settings->value("sprout_hires_camera_pid", 0xF580).toInt()) {
                m_camera.reset(new proapi::camera::Camera(device.index));
            }

            if (device.name == "depthcamera" &&
                device.vendor_id == settings->value("sprout_depth_camera_vid", 0x2BC5).toInt() &&
                device.product_id == settings->value("sprout_depth_camera_pid", 0x0405).toInt()) {
                m_model->liveCapture()->setDepthCameraIndex(device.index);
            }
        }

        if (m_camera.isNull()) {
            qCritical() << "No Sprout hires camera was found on the system!";
        }
    }

    connect(liveCaptureModel.data(), &model::LiveCaptureModel::videoStreamSourcesChanged, this, &VideoSourceManager::onVideoStreamSourcesChanged);
    onVideoStreamSourcesChanged(m_model->liveCapture()->videoStreamSources());
}

void VideoSourceManager::verifySelectedCameras() {
    auto liveCapture = m_model->liveCapture();
    auto selectedVideoStreamSources = liveCapture->selectedVideoStreamSources();

    // Remove unavailable sources
    for (auto selectedVideoStreamSource : selectedVideoStreamSources) {
        bool isAvailable = false;
        for (auto videoStreamSource : liveCapture->videoStreamSources()) {
            isAvailable = videoStreamSource->videoSource() == selectedVideoStreamSource ? true : isAvailable;
        }

        if (!isAvailable) {
            qInfo() << this << "Removing source" << selectedVideoStreamSource << "from selected sources";
            selectedVideoStreamSources.removeOne(selectedVideoStreamSource);
        }
    }

    // If we have empty list try to find camera that we can switch to
    if (selectedVideoStreamSources.count() == 0) {
        if (m_model->singleScreenMode()) {
            if (isSproutCameraConnected()) {
                selectedVideoStreamSources << common::VideoSourceInfo::SproutCamera();
            } else {
                // Check if some webcam is attached
                QSharedPointer<model::VideoStreamSourceModel> webcam;

                for (auto streamSource : m_model->liveCapture()->videoStreamSources()) {
                    webcam = streamSource->videoSource().type == common::VideoSourceInfo::SourceType::Webcamera ? streamSource : webcam;
                }

                if (!webcam.isNull()) {
                    selectedVideoStreamSources << webcam->videoSource();
                }
            }
        } else {
            selectedVideoStreamSources << common::VideoSourceInfo::DownwardFacingCamera();
        }
    }

    liveCapture->setSelectedVideoStreamSources(selectedVideoStreamSources);
}

void VideoSourceManager::onUsbDeviceConnectionChanged() {
    auto settings = GlobalUtilities::applicationSettings("camera_manager");

    QTimer::singleShot(settings->value("detection_timeout", 1000).toInt(), [this] {
        findAttachedWebcameras();
        onDeviceConnectionChanged();
    });
}

void VideoSourceManager::findAttachedCameraDevices() {
    m_sohalDevices = m_system->deviceIDs();

    qInfo() << this << "Following SOHAL devices were found:";
    for (auto device : m_sohalDevices) {
        qInfo() << this << device.name << device.index << QByteArray::number(device.vendor_id, 16) << QByteArray::number(device.product_id, 16);
    }
}

void VideoSourceManager::findAttachedWebcameras() {
    QStringList defaultIgnoredCameras ( { "HP 14MP Camera", // Hurley
                                          "USB Camera-OV580", // Gen1/2 downward facing camera
                                          "Fortis Video Source", // Gen2 virtual camera
                                          "Downward Facing camera", // Gen1 virtual camera
                                          "HP 2.0MP High Definition Webcam", // Gen2 forward facing camera
                                          "Forward Facing camera", // Gen1 virtual camera
                                          "Intel(R) RealSense(TM) 3D Camera Virtual Driver" // Gen1/2 depth camera
                                      } );

    auto ignoredCameras = GlobalUtilities::applicationSettings("camera_manager")->value("ignored_camera_names", defaultIgnoredCameras).toStringList();
    QVector<QCameraInfo> attachedCameras;

    for(auto cameraInfo : QCameraInfo::availableCameras()) {
        qDebug() << this << "Found camera" << cameraInfo.description();
        if (!ignoredCameras.contains(cameraInfo.description())) {
            attachedCameras << cameraInfo;
        }
    }

    m_webcameras = attachedCameras;

    qInfo() << this << "Following webcameras were found:";
    for (auto device : m_webcameras) {
        qInfo() << this << device.description() << device.deviceName();
    }
}

void VideoSourceManager::onDeviceConnected(proapi::hal::DeviceID deviceID) {
    if (m_sohalDevices.indexOf(deviceID) < 0) {
        m_sohalDevices << deviceID;
    }
    findAttachedWebcameras();
    onDeviceConnectionChanged();
}

void VideoSourceManager::onDeviceDisconnected(proapi::hal::DeviceID deviceID) {
    m_sohalDevices.removeOne(deviceID);
    findAttachedWebcameras();
    onDeviceConnectionChanged();
}

void VideoSourceManager::updateColorCorrection()
{
    auto liveCaptureModel = m_model->liveCapture();

    for(auto videoStreamSource : liveCaptureModel->videoStreamSources()) {
        updateCameraColorCorrection(videoStreamSource);
    }
}

void VideoSourceManager::onMatModeStateChanged(model::ApplicationStateModel::MatModeState state) {
    if (m_model->mode() == model::ApplicationStateModel::Mode::LiveCapture &&
        m_model->liveCapture()->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running) {
        switch(state) {
        case model::ApplicationStateModel::MatModeState::None:
        case model::ApplicationStateModel::MatModeState::Reprojection:
        case model::ApplicationStateModel::MatModeState::TransitioningToNone:
        case model::ApplicationStateModel::MatModeState::TransitioningToDesktop:
        case model::ApplicationStateModel::MatModeState::TransitioningToLampOff:
        case model::ApplicationStateModel::MatModeState::TransitioningToLampOn:
        case model::ApplicationStateModel::MatModeState::TransitioningToFlash:
        case model::ApplicationStateModel::MatModeState::TransitioningToReprojection:
            // No action
            break;
        case model::ApplicationStateModel::MatModeState::Desktop:
        case model::ApplicationStateModel::MatModeState::LampOff:
            if (m_model->singleScreenMode()) {
                setSproutCameraStreamingLEDState(false);
            } else {
                setDownwardFacingCameraParameters(proapi::camera::CameraSettings::Auto, false, false);
            }
            break;
        case model::ApplicationStateModel::MatModeState::LampOn:
            if (m_model->singleScreenMode()) {
                setSproutCameraStreamingLEDState(true);
            } else {
                setDownwardFacingCameraParameters(proapi::camera::CameraSettings::Auto, false, false);
            }
            break;
        case model::ApplicationStateModel::MatModeState::Flash:
            setDownwardFacingCameraParameters(proapi::camera::CameraSettings::Default4K, true, true);
            break;
        default:
            Q_UNREACHABLE();
        }

        updateColorCorrection();
    }
}

void VideoSourceManager::setDownwardFacingCameraParameters(proapi::camera::CameraSettings::Mode mode,
                                                                  bool illuminationCorrection, bool autoWhiteBalance) {    
    auto downwardFacingSourceModel = m_model->liveCapture()->videoStreamSource(common::VideoSourceInfo::DownwardFacingCamera());

    if (!downwardFacingSourceModel.isNull()) {
        downwardFacingSourceModel->setIlluminationCorrectionEnabled(illuminationCorrection);
        downwardFacingSourceModel->setAutoWhiteBalanceEnabled(autoWhiteBalance);
    }

    proapi::camera::CameraSettings settings;
    settings.options |= proapi::camera::CameraSettings::Exposure;
    settings.exposure_mode = mode;

    settings.options |= proapi::camera::CameraSettings::Gain;
    settings.gain_mode = mode;

    settings.options |= proapi::camera::CameraSettings::WhiteBalance;
    settings.white_balance_mode = mode;

    settings.options |= proapi::camera::CameraSettings::GammaCorrection;
    settings.gamma_correction = true;

    settings.options |= proapi::camera::CameraSettings::LensColorShading;
    settings.lens_color_shading = true;

    settings.options |= proapi::camera::CameraSettings::LensShading;
    settings.lens_shading = true;

    if (m_camera) {
        m_camera->setCameraSettings(settings);
    }
}

void VideoSourceManager::setSproutCameraStreamingLEDState(bool streamingLEDState) {
    if (m_camera) {
        m_camera->setLEDState({ "auto", streamingLEDState ? "low" : "off" });
    }
}

void VideoSourceManager::updateCameraColorCorrection(QSharedPointer<model::VideoStreamSourceModel> camera) {
    auto liveCaptureModel = m_model->liveCapture();
    const auto autoFix = liveCaptureModel->autoFix();

    auto disableColorCorrection = true;

    if (liveCaptureModel->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running) {
        if (autoFix) {
            switch (m_model->matModeState()) {
            case model::ApplicationStateModel::MatModeState::LampOn:
            case model::ApplicationStateModel::MatModeState::TransitioningToLampOn:
                camera->setColorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode::LampOn);
                disableColorCorrection = false;
                break;
            case model::ApplicationStateModel::MatModeState::LampOff:
            case model::ApplicationStateModel::MatModeState::TransitioningToLampOff:
                camera->setColorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode::LampOff);
                disableColorCorrection = false;
                break;
            }
        }
    }

    if (disableColorCorrection) {
        camera->setColorCorrectionMode(model::VideoStreamSourceModel::ColorCorrectionMode::None);
    }
}

void VideoSourceManager::onVideoStreamSourcesChanged(QVector<QSharedPointer<model::VideoStreamSourceModel>> videoStreamSources) {
    bool haveVideoCameraDevice = false;

    // If none of these are found we want to display warning to the user
    static QVector<common::VideoSourceInfo::SourceType> videoSources ({ common::VideoSourceInfo::SourceType::DownwardFacingCamera,
                                                                        common::VideoSourceInfo::SourceType::ForwardFacingCamera,
                                                                        common::VideoSourceInfo::SourceType::SproutCamera,
                                                                        common::VideoSourceInfo::SourceType::Webcamera });

    for (auto videoStreamSource : videoStreamSources) {
        haveVideoCameraDevice = videoSources.contains(videoStreamSource->videoSource().type) ? true : haveVideoCameraDevice;
    }

    if(m_model->mode() != model::ApplicationStateModel::Mode::Preview)
        m_model->setMode(haveVideoCameraDevice ? model::ApplicationStateModel::Mode::LiveCapture : model::ApplicationStateModel::Mode::NoVideoSource);

    verifySelectedCameras();
}

void VideoSourceManager::onCameraStateChanged() {
    static QMap<model::LiveCaptureModel::VideoStreamState, model::ApplicationStateModel::Mode> transitionMap = {
       { model::LiveCaptureModel::VideoStreamState::FailedToStart, model::ApplicationStateModel::Mode::CameraFailedToStart },
       { model::LiveCaptureModel::VideoStreamState::CalibrationDataMissing, model::ApplicationStateModel::Mode::NoCalibrationData },
       { model::LiveCaptureModel::VideoStreamState::Running, model::ApplicationStateModel::Mode::LiveCapture }
    };

    auto videoStreamState = m_model->liveCapture()->videoStreamState();

    if (transitionMap.contains(videoStreamState)) {
        m_model->setMode(transitionMap[videoStreamState]);
    }

    updateColorCorrection();
}

void VideoSourceManager::onApplicationModeChanged(model::ApplicationStateModel::Mode mode) {
    if (mode == model::ApplicationStateModel::Mode::LiveCapture) {
        startCurrentlySelectedCameras();
    } else if (mode != model::ApplicationStateModel::Mode::KeystoneCorrectionCalibration) {
        stopAllCameras();
    }
}

void VideoSourceManager::startCurrentlySelectedCameras() {
    onSelectedVideoStreamSourcesChanged(m_model->liveCapture()->selectedVideoStreamSources());
}

void VideoSourceManager::stopAllCameras() {
    auto stopStreamingEvent = new event::StopVideoStreamingEvent();
    stopStreamingEvent->dispatch();
}

void VideoSourceManager::onSelectedVideoStreamSourcesChanged(QVector<common::VideoSourceInfo> selectedCameras) {
    if (selectedCameras.count() > 0) {
        auto startStreamingEvent = new event::StartVideoStreamingEvent(selectedCameras);
        startStreamingEvent->dispatch();
    }
}

QPair<int, int> VideoSourceManager::sproutCameraIndices() {
    int hurleyDepthCameraIndex = -1, hurleyHiresCameraIndex = -1;
    auto settings = GlobalUtilities::applicationSettings("camera_manager");

    for (auto device : m_sohalDevices) {
        if (device.name == "depthcamera" && device.vendor_id == settings->value("hurley_depth_camera_vid", 0x02BC5).toInt() &&
                device.product_id == settings->value("hurley_depth_camera_pid", 0x0406).toInt()) {
            hurleyDepthCameraIndex = device.index;
        }

        if (device.name == "hirescamera" && device.vendor_id == settings->value("hurley_hires_camera_vid", 0x05C8).toInt() &&
                               device.product_id == settings->value("hurley_hires_camera_pid", 0xF582).toInt()) {
            hurleyHiresCameraIndex = device.index;
        }
    }

    return qMakePair(hurleyHiresCameraIndex, hurleyDepthCameraIndex);
}

bool VideoSourceManager::isSproutCameraConnected() {
    const auto indices = sproutCameraIndices();
    auto settings = GlobalUtilities::applicationSettings("camera_manager");

    return (indices.first >= 0 || settings->value("hurley_hires_camera_override", false).toBool()) &&
           (indices.second >= 0 || settings->value("hurley_depth_camera_override", false).toBool());
}

void VideoSourceManager::checkVideoSourcePresence(common::VideoSourceInfo videoSource, QVector<QSharedPointer<model::VideoStreamSourceModel>>* videoStreamSources) {
    auto cameraIndex = -1;

    for (int i = 0; i < videoStreamSources->length(); i++) {
        if (videoStreamSources->at(i)->videoSource() == videoSource) {
            cameraIndex = i;
            break;
        }
    }

    auto isCameraConnected = false;
    const auto isSproutCamera = videoSource == common::VideoSourceInfo::SproutCamera();

    if (isSproutCamera) {
        isCameraConnected = isSproutCameraConnected();
    } else if (videoSource.type == common::VideoSourceInfo::SourceType::Webcamera) {
        for(auto cameraInfo : m_webcameras) {
            isCameraConnected = videoSource.name == cameraInfo.description() ? true : isCameraConnected;
        }
    }

    if (isCameraConnected) {
        if (cameraIndex < 0) {
            qInfo() << this << "New video source" << videoSource << "detected, adding to the list of detected devices";
            videoStreamSources->push_back(QSharedPointer<model::VideoStreamSourceModel>::create(videoSource));
        }

        // Also create SOHAL device if this is Sprout camera
        if (isSproutCamera) {
            const auto indices = sproutCameraIndices();

            if (m_camera.isNull() || m_camera->info().index != indices.first) {
                m_camera.reset(new proapi::camera::Camera(indices.first));

                m_model->liveCapture()->setDepthCameraIndex(indices.second);
                onKeystoneCorrectionModeChanged(m_model->liveCapture()->keystoneCorrectionMode());

                // ToDo: we should read out keystone coefficients for Sprout camera from calibration package
                // but we don't have it yet so just read values from configuration file

                auto settings = GlobalUtilities::applicationSettings("sprout_camera_calibration");
                auto keystoneCalibration = m_model->keystoneCalibration();
                keystoneCalibration->setTopLeft(settings->value("cropped_top_left", QPoint(600, 300)).toPoint());
                keystoneCalibration->setTopRight(settings->value("cropped_top_right", QPoint(-600, 300)).toPoint());
                keystoneCalibration->setBottomLeft(settings->value("cropped_bottom_left", QPoint(200, -400)).toPoint());
                keystoneCalibration->setBottomRight(settings->value("cropped_bottom_right", QPoint(-200, -400)).toPoint());
            }
        }
    } else if (cameraIndex >= 0) {
        qInfo() << this << "Video source" << videoSource << "no longer detected, removing from the list of detected devices";
        videoStreamSources->removeAt(cameraIndex);
    }
}

void VideoSourceManager::onDeviceConnectionChanged() {
    auto liveCapture = m_model->liveCapture();
    auto videoStreamSources = liveCapture->videoStreamSources();

    checkVideoSourcePresence(common::VideoSourceInfo::SproutCamera(), &videoStreamSources);

    // Clean up webcams and then refill with current devices
    for (int i = videoStreamSources.length() - 1; i >= 0; i--)  {
        if (videoStreamSources.at(i)->videoSource().type == common::VideoSourceInfo::SourceType::Webcamera) {
            videoStreamSources.removeAt(i);
        }
    }

    for (auto cameraInfo : m_webcameras) {
        common::VideoSourceInfo videoSource (common::VideoSourceInfo::SourceType::Webcamera, cameraInfo.description());

        QCamera camera(cameraInfo);

        camera.load();

        auto preferredSettings = camera.supportedViewfinderSettings().first();

        camera.unload();

        videoSource.resolution = preferredSettings.resolution();
        videoSource.frameRate = static_cast<unsigned int>(preferredSettings.maximumFrameRate());

        checkVideoSourcePresence(videoSource, &videoStreamSources);
    }    

    liveCapture->setVideoStreamSources(videoStreamSources);
}


void VideoSourceManager::onPicInPicModeChanged(bool picInPicMode) {
    if (!picInPicMode) {
        auto liveCapture = m_model->liveCapture();
        auto selectedVideoStreamSources = liveCapture->selectedVideoStreamSources();

        // Make sure that just one source is selected
        while (selectedVideoStreamSources.count() > 1) {
            selectedVideoStreamSources.removeLast();
        }

        liveCapture->setSelectedVideoStreamSources(selectedVideoStreamSources);
    }
}

void VideoSourceManager::onDisplayCountChanged(int count) {
    const auto liveCapture = m_model->liveCapture();
    auto videoStreamSources = liveCapture->videoStreamSources();

    // Remove all Desktop sources

    for (int i = videoStreamSources.length() - 1; i >= 0; i--) {
        const auto type = videoStreamSources.at(i)->videoSource().type;
        if (type == common::VideoSourceInfo::SourceType::PrimaryDesktop || type == common::VideoSourceInfo::SourceType::MatDesktop) {
            videoStreamSources.removeAt(i);
        }
    }

    if (count >= 1) {
        videoStreamSources.push_back(QSharedPointer<model::VideoStreamSourceModel>::create(common::VideoSourceInfo::PrimaryDesktop()));
    }

    auto settings = GlobalUtilities::applicationSettings("camera_manager");

    if (count >= settings->value("mat_desktop_display_treshold", 2).toInt()) {
        videoStreamSources.push_back(QSharedPointer<model::VideoStreamSourceModel>::create(common::VideoSourceInfo::MatDesktop()));
    }

    liveCapture->setVideoStreamSources(videoStreamSources);
}

void VideoSourceManager::onKeystoneCorrectionModeChanged(model::LiveCaptureModel::KeystoneCorrectionMode mode) {
    if (m_camera && m_model->singleScreenMode()) {
        // ToDo:: temporary read keystone correction points from configuration file instead of calibration

        const proapi::camera::CameraResolution resolution { 6, 4351, 3263 };
        auto settings = GlobalUtilities::applicationSettings("sprout_camera_calibration");

        proapi::camera::CameraKeystone keystone = m_camera->keystone();

        switch(mode) {
        case model::LiveCaptureModel::KeystoneCorrectionMode::NoKeystoneCorrection:
            keystone.enabled = false;
            break;
        case model::LiveCaptureModel::KeystoneCorrectionMode::CroppedKeystoneCorrection: {
            const auto keystoneCalibration = m_model->keystoneCalibration();

            const proapi::camera::CameraQuadrilateral quadrilateral {
                { keystoneCalibration->bottomLeft().x(), keystoneCalibration->bottomLeft().y() },
                { keystoneCalibration->bottomRight().x(), keystoneCalibration->bottomRight().y() },
                { keystoneCalibration->topLeft().x(), keystoneCalibration->topLeft().y() },
                { keystoneCalibration->topRight().x(), keystoneCalibration->topRight().y() }
            };

            keystone.enabled = true;
            keystone.capture = quadrilateral;
            keystone.streaming = quadrilateral;

            break;
        }
        case model::LiveCaptureModel::KeystoneCorrectionMode::NonCroppedKeystoneCorrection: {
            const auto topLeft = settings->value("noncropped_top_left", QPoint(200, 0)).toPoint();
            const auto topRight = settings->value("noncropped_top_right", QPoint(-200, 0)).toPoint();
            const auto bottomLeft = settings->value("noncropped_bottom_left", QPoint(100, 0)).toPoint();
            const auto bottomRight = settings->value("noncropped_bottom_right", QPoint(-100, 0)).toPoint();

            const proapi::camera::CameraQuadrilateral quadrilateral {
                { bottomLeft.x(), bottomLeft.y() },
                { bottomRight.x(), bottomRight.y() },
                { topLeft.x(), topLeft.y() },
                { topRight.x(), topRight.y() }
            };

            keystone.enabled = true;
            keystone.capture = quadrilateral;
            keystone.streaming = quadrilateral;

            break;
        }

        }

        if (keystone.enabled) {
            m_camera->setKeystone(keystone);
        } else {
            m_camera->setKeystoneEnabled(false);
        }
    }
}

} // namespace components
} // namespace capture
