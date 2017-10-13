#pragma once
#ifndef APPLICATIONSTATEMODEL_H
#define APPLICATIONSTATEMODEL_H

#include <QObject>
#include <QSharedPointer>

#include "live_capture_model.h"
#include "observable_stage_project_collection.h"
#include "post_capture_model.h"
#include "projects_export_model.h"
#include "keystone_calibration_model.h"

namespace capture {
namespace model {

class ApplicationStateModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QSharedPointer<model::LiveCaptureModel> liveCapture READ liveCapture CONSTANT)
    Q_PROPERTY(QSharedPointer<PostCaptureModel> postCapture READ postCapture CONSTANT)
    Q_PROPERTY(QSharedPointer<model::ProjectsExportModel> projectsExport READ projectsExport CONSTANT)
    Q_PROPERTY(model::ApplicationStateModel::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QSharedPointer<ObservableStageProjectCollection> projects READ projects CONSTANT)
    Q_PROPERTY(QSharedPointer<KeystoneCalibrationModel> keystoneCalibration READ keystoneCalibration CONSTANT)
    Q_PROPERTY(QSharedPointer<StageProject> selectedProject READ selectedProject WRITE
               setSelectedProject NOTIFY selectedProjectChanged)
    Q_PROPERTY(model::ApplicationStateModel::MatModeState matModeState READ matModeState WRITE
               setMatModeState NOTIFY matModeStateChanged)
    Q_PROPERTY(bool presentationMode READ presentationMode WRITE setPresentationMode NOTIFY
               presentationModeChanged)
    Q_PROPERTY(model::ApplicationStateModel::MainWindowLocation mainWindowLocation READ mainWindowLocation
               WRITE setMainWindowLocation NOTIFY mainWindowLocationChanged)
    Q_PROPERTY(bool monitorWindowMinimized READ monitorWindowMinimized WRITE setMonitorWindowMinimized
               NOTIFY monitorWindowMinimizedChanged)
    Q_PROPERTY(bool applicationSuspended READ applicationSuspended WRITE setApplicationSuspended
               NOTIFY applicationSuspendedChanged)
    Q_PROPERTY(EditMenuMode editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)
    Q_PROPERTY(bool singleScreenMode READ singleScreenMode WRITE setSingleScreenMode
               NOTIFY singleScreenModeChanged)
    Q_PROPERTY(bool countDownTimerState READ countDownTimerState WRITE setCountDownTimerState NOTIFY countDownTimerStateChanged)
    Q_PROPERTY(bool sendToStageMode READ sendToStageMode WRITE setSendToStageMode NOTIFY sendToStageModeChanged)
    Q_PROPERTY(bool picInPicMode READ picInPicMode WRITE setPicInPicMode
               NOTIFY picInPicModeChanged)
    Q_PROPERTY(QPixmap inkPixMap READ inkPixMap WRITE setInkPixMap NOTIFY inkPixMapChanged)
    Q_PROPERTY(model::ApplicationStateModel::ColorCalibrationStatus colorCalibrationStatus READ colorCalibrationStatus
               WRITE setColorCalibrationStatus NOTIFY colorCalibrationStatusChanged)    
    Q_PROPERTY(double inkPenWidthScale READ inkPenWidthScale WRITE setInkPenWidthScale)

public:
    explicit ApplicationStateModel(QObject *parent = 0);

    enum class Mode {
        None,
        LiveCapture,
        Preview,
        CameraFailedToStart,
        NoCalibrationData,
        NoVideoSource,
        ColorCorrectionCalibration,
        KeystoneCorrectionCalibration
    };

    Q_ENUM(Mode)

    enum MatModeState {
        None,
        TransitioningToLampOff,
        LampOff,
        TransitioningToLampOn,
        LampOn,
        TransitioningToDesktop,
        Desktop,
        TransitioningToFlash,
        Flash,
        TransitioningToReprojection,
        Reprojection,
        TransitioningToNone
    };

    Q_ENUM(MatModeState)

    enum class MainWindowLocation { None, MonitorOnMat, MonitorOnExtend };

    Q_ENUM(MainWindowLocation)

    enum class EditMenuMode {None, SubMenuOpen, SubMenuClose, MainMenuOpen, MainMenuClose };

    Q_ENUM(EditMenuMode)

    enum class ColorCalibrationStatus { NotCalibrating, CapturingImages, PerformingCalibration, CalibrationComplete };

    Q_ENUM(ColorCalibrationStatus)

    inline QSharedPointer<model::LiveCaptureModel> liveCapture() const { return m_liveCaptureModel; }
    inline QSharedPointer<PostCaptureModel> postCapture() const { return m_postCaptureModel; }
    inline QSharedPointer<model::ProjectsExportModel> projectsExport() const { return m_projectsExport; }
    inline QSharedPointer<ObservableStageProjectCollection> projects() const { return m_projects; }
    inline QSharedPointer<KeystoneCalibrationModel> keystoneCalibration() const { return m_keystoneCalibration; }
    inline ApplicationStateModel::Mode mode() const { return m_mode; }
    inline ApplicationStateModel::MatModeState matModeState() const { return m_matModeState; }
    inline MainWindowLocation mainWindowLocation() const { return m_mainWindowLocation; }
    inline bool presentationMode() const { return m_presentationMode; }
    inline bool monitorWindowMinimized() const { return m_monitorWindowMinimized; }
    inline bool applicationSuspended() const { return m_applicationSuspended; }
    inline EditMenuMode editMode() const { return m_editMode; }
    inline bool singleScreenMode() const { return m_singleScreenMode; }
    inline bool countDownTimerState() const { return m_countDownTimerState; }
    inline bool sendToStageMode() const { return m_sendToStageMode; }
    inline bool sendToStageModeCapture() const { return m_sendToStageModeCapture; }
    inline bool picInPicMode() const { return m_picInPicMode; }
    inline bool isEditMode() const { return m_editMode == model::ApplicationStateModel::EditMenuMode::SubMenuOpen; }
    inline QPixmap inkPixMap() const { return m_inkPixmap; }
    inline ApplicationStateModel::ColorCalibrationStatus colorCalibrationStatus() const { return m_colorCalibrationStatus; }    
    inline double inkPenWidthScale() const {return m_inkPenWidthScale; }

    QSharedPointer<StageProject> selectedProject() const;
    bool isInTransitionalMatMode() const;

signals:
    void modeChanged(capture::model::ApplicationStateModel::Mode mode);
    void selectedProjectChanged(QSharedPointer<StageProject> selectedProject);
    void matModeStateChanged(capture::model::ApplicationStateModel::MatModeState state, capture::model::ApplicationStateModel::MatModeState oldState);
    void presentationModeChanged(bool presentationMode);
    void mainWindowLocationChanged(MainWindowLocation location);
    void monitorWindowMinimizedChanged(bool monitorWindowMinimized);
    void applicationSuspendedChanged(bool applicationSuspended);
    void inkWidgetChanged();
    void editModeChanged(EditMenuMode mode);
    void editButtonChanged(bool checked);
    void singleScreenModeChanged(bool singleScreenMode);
    void countDownTimerStateChanged(bool checked);
    void sendToStageModeChanged(bool autoMode);
    void picInPicModeChanged(bool picInPicMode);
    void inkPixMapChanged(QPixmap inkPixMap);
    void colorCalibrationStatusChanged(capture::model::ApplicationStateModel::ColorCalibrationStatus colorCalibrationStatus);

public slots:
    void setMode(capture::model::ApplicationStateModel::Mode mode);
    void setSelectedProject(QSharedPointer<StageProject> selectedProject);
    void setPresentationMode(bool isPresent);
    void setMainWindowLocation(MainWindowLocation location);
    void setMonitorWindowMinimized(bool monitorWindowMinimized);
    void setApplicationSuspended(bool applicationSuspended);
    void setSingleScreenMode(bool singleScreenMode);
    void setMatModeState(capture::model::ApplicationStateModel::MatModeState matMode);
    void setEditMode(EditMenuMode mode);
    void setCountDownTimerState(bool state);
    void setSendToStageMode(bool autoMode);
    void setSendToStageModeCapture(bool sendToStageModeCapture);
    void setPicInPicMode(bool picInPicMode);
    void setInkPixMap(QPixmap inkPixMap);
    void setColorCalibrationStatus(capture::model::ApplicationStateModel::ColorCalibrationStatus colorCalibrationStatus);
    void setInkPenWidthScale(double inkPenWidthScale);

private:
    int m_selectedProjectIndex;
    QSharedPointer<model::LiveCaptureModel> m_liveCaptureModel;
    QSharedPointer<ObservableStageProjectCollection> m_projects;
    QSharedPointer<PostCaptureModel> m_postCaptureModel;
    QSharedPointer<model::ProjectsExportModel> m_projectsExport;
    QSharedPointer<KeystoneCalibrationModel> m_keystoneCalibration;
    ApplicationStateModel::Mode m_mode;
    ApplicationStateModel::MatModeState m_matModeState;
    MainWindowLocation m_mainWindowLocation;
    bool m_presentationMode;
    bool m_monitorWindowMinimized;
    bool m_applicationSuspended;
    EditMenuMode m_editMode;
    bool m_singleScreenMode;
    bool m_countDownTimerState;
    bool m_sendToStageMode;
    bool m_sendToStageModeCapture;
    bool m_picInPicMode;
    double m_inkPenWidthScale;
    QPixmap m_inkPixmap;
    ColorCalibrationStatus m_colorCalibrationStatus;    
};

} // namespace model
} // namespace capture

Q_DECLARE_METATYPE(capture::model::ApplicationStateModel::Mode)
Q_DECLARE_METATYPE(capture::model::ApplicationStateModel::MatModeState)

#endif  // APPLICATIONSTATEMODEL_H
