#pragma once
#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QCameraInfo>

#include <hal/system.h>
#include <camera/camera.h>
#include <user_event_handler.h>

#include "model/application_state_model.h"

namespace capture {
namespace components {

/*!
 * \brief The VideoSourceManager class is responsible for starting and stopping cameras when application is transitioning between various states.
 */
class VideoSourceManager : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief VideoSourceManager constructor.
     * \param model Application model.
     * \param parent Parent object.
     */
    explicit VideoSourceManager(QSharedPointer<model::ApplicationStateModel> model, QObject *parent = nullptr);

private slots:

    void onApplicationModeChanged(model::ApplicationStateModel::Mode mode);
    void onCameraStateChanged();
    void updateColorCorrection();
    void onSelectedVideoStreamSourcesChanged(QVector<capture::common::VideoSourceInfo> selectedCameras);
    void onDeviceConnectionChanged();
    void onUsbDeviceConnectionChanged();
    void onMatModeStateChanged(capture::model::ApplicationStateModel::MatModeState state);
    void onDeviceConnected(proapi::hal::DeviceID deviceID);
    void onDeviceDisconnected(proapi::hal::DeviceID deviceID);
    void onVideoStreamSourcesChanged(QVector<QSharedPointer<capture::model::VideoStreamSourceModel>> videoStreamSources);
    void onPicInPicModeChanged(bool picInPicMode);
    void onDisplayCountChanged(int count);
    void onKeystoneCorrectionModeChanged(capture::model::LiveCaptureModel::KeystoneCorrectionMode mode);

protected:

    void stopAllCameras();
    void startCurrentlySelectedCameras();
    void updateCameraColorCorrection(QSharedPointer<model::VideoStreamSourceModel> camera);
    void findAttachedCameraDevices();
    void findAttachedWebcameras();
    void setDownwardFacingCameraParameters(proapi::camera::CameraSettings::Mode mode, bool illuminationCorrection, bool autoWhiteBalance);
    void setSproutCameraStreamingLEDState(bool streamingLEDState);
    QPair<int, int> sproutCameraIndices();
    bool isSproutCameraConnected();
    void checkVideoSourcePresence(capture::common::VideoSourceInfo videoSource, QVector<QSharedPointer<model::VideoStreamSourceModel>> *videoStreamSources);
    void verifySelectedCameras();

    QSharedPointer<model::ApplicationStateModel> m_model;
    QScopedPointer<proapi::hal::System> m_system;
    QScopedPointer<proapi::camera::Camera> m_camera;
    QScopedPointer<user_event_handler::EventHandler> m_eventHandler;

    QVector<proapi::hal::DeviceID> m_sohalDevices;
    QVector<QCameraInfo> m_webcameras;
};

} // namespace components
} // namespace capture

#endif // CAMERAMANAGER_H
