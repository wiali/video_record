#include "application_state_model.h"

#include <QDebug>

#include <global_utilities.h>

#include "common/utilities.h"

namespace capture {
namespace model {

ApplicationStateModel::ApplicationStateModel(QObject *parent)
    : QObject(parent)    
    , m_selectedProjectIndex(-1)
    , m_liveCaptureModel(new LiveCaptureModel)
    , m_projects(new ObservableStageProjectCollection)
    , m_postCaptureModel(new PostCaptureModel)    
    , m_projectsExport(new ProjectsExportModel)
    , m_keystoneCalibration(new KeystoneCalibrationModel)
    , m_mode(Mode::None)
    , m_matModeState(MatModeState::None)
    , m_mainWindowLocation(MainWindowLocation::None)
    , m_presentationMode(false)
    , m_monitorWindowMinimized(false)
    , m_applicationSuspended(false)
    , m_editMode(EditMenuMode::None)
    , m_singleScreenMode(false)
    , m_sendToStageMode(false)
    , m_sendToStageModeCapture(false)
    , m_colorCalibrationStatus(ColorCalibrationStatus::NotCalibrating)    
    , m_inkPenWidthScale(1.0)
{
    // Needed for proper cross-thread signals
    static struct Initialize {
        Initialize() {
            qRegisterMetaType<capture::model::ApplicationStateModel::Mode>();
            qRegisterMetaType<capture::model::ApplicationStateModel::MatModeState>();
            qRegisterMetaType<capture::model::ApplicationStateModel::MainWindowLocation>();
            qRegisterMetaType<capture::model::ApplicationStateModel::EditMenuMode>();
            qRegisterMetaType<capture::model::ApplicationStateModel::ColorCalibrationStatus>();
        }
    } initialize;

    auto settings = GlobalUtilities::applicationSettings("capture_button");
    m_countDownTimerState = settings->value("countdown_timer_state", true).toBool();
    m_picInPicMode = settings->value("picture_in_picture", false).toBool();
}

QSharedPointer<StageProject> ApplicationStateModel::selectedProject() const {
    QSharedPointer<StageProject> result;

    if (m_selectedProjectIndex >= 0 && m_selectedProjectIndex < projects()->count()) {
        result = projects()->at(m_selectedProjectIndex);
    }

    return result;
}

void ApplicationStateModel::setSelectedProject(QSharedPointer<StageProject> selectedProject) {
    const auto index = projects()->items().indexOf(selectedProject);
    if (index >= 0) {
        qInfo() << this << "Selecting project index" << index;
        m_selectedProjectIndex = index;

        emit selectedProjectChanged(selectedProject);
    } else {
        qWarning() << this << "Provided stage project is not in projects collection, cannot select";
    }
}

void ApplicationStateModel::setPresentationMode(bool presentationMode) {
    if (m_presentationMode != presentationMode) {
        m_presentationMode = presentationMode;
        qInfo() << this << "Changing presentation mode to" << m_presentationMode;

        emit presentationModeChanged(m_presentationMode);
    }
}

void ApplicationStateModel::setMainWindowLocation(MainWindowLocation location) {
    if (m_mainWindowLocation != location) {
        m_mainWindowLocation = location;

        qInfo() << this << "Changing main window location to" << location;

        emit mainWindowLocationChanged(m_mainWindowLocation);
    }
}

void ApplicationStateModel::setMode(model::ApplicationStateModel::Mode mode) {
    if (m_mode != mode) {
        m_mode = mode;
        qInfo() << this << "Changing mode to" << m_mode;

        emit modeChanged(m_mode);
    }
}

void ApplicationStateModel::setMatModeState(model::ApplicationStateModel::MatModeState matMode) {
    if (m_matModeState != matMode) {
        const auto oldState = m_matModeState;

        m_matModeState = matMode;
        qInfo() << this << "Changing mat mode from" << oldState << "to" << m_matModeState;

        emit matModeStateChanged(m_matModeState, oldState);
    }
}

bool ApplicationStateModel::isInTransitionalMatMode() const {
    static QHash<ApplicationStateModel::MatModeState, bool> isInTransitionalMatModeMap {
        { ApplicationStateModel::MatModeState::LampOff, false },
        { ApplicationStateModel::MatModeState::LampOn, false },
        { ApplicationStateModel::MatModeState::Desktop, false },
        { ApplicationStateModel::MatModeState::Flash, false },
        { ApplicationStateModel::MatModeState::Reprojection, false },
        { ApplicationStateModel::MatModeState::None, false },
        { ApplicationStateModel::MatModeState::TransitioningToLampOff, true },
        { ApplicationStateModel::MatModeState::TransitioningToLampOn, true },
        { ApplicationStateModel::MatModeState::TransitioningToDesktop, true },
        { ApplicationStateModel::MatModeState::TransitioningToFlash, true },
        { ApplicationStateModel::MatModeState::TransitioningToReprojection, true },
        { ApplicationStateModel::MatModeState::TransitioningToNone, true }
    };

    return isInTransitionalMatModeMap[m_matModeState];
}

void ApplicationStateModel::setMonitorWindowMinimized(bool monitorWindowMinimized) {
    if (m_monitorWindowMinimized != monitorWindowMinimized) {
        m_monitorWindowMinimized = monitorWindowMinimized;

        qInfo() << this << "Monitor window minimize state change to" << m_monitorWindowMinimized;

        emit monitorWindowMinimizedChanged(m_monitorWindowMinimized);
    }
}

void ApplicationStateModel::setApplicationSuspended(bool applicationSuspended) {
    if (m_applicationSuspended != applicationSuspended) {
        m_applicationSuspended = applicationSuspended;

        qInfo() << this << "Application suspended changed to" << m_applicationSuspended;

        emit applicationSuspendedChanged(m_applicationSuspended);
    }
}

void ApplicationStateModel::setEditMode(EditMenuMode mode) {
    if (m_editMode != mode) {
        m_editMode = mode;

        qInfo() << this << "Application edit mode changed to" << m_editMode;

        emit editModeChanged(m_editMode);
    }
}

void ApplicationStateModel::setSingleScreenMode(bool singleScreenMode) {
    if (m_singleScreenMode != singleScreenMode) {
        m_singleScreenMode = singleScreenMode;

        qInfo() << this << "Single screen mode changed to" << m_singleScreenMode;

        emit singleScreenModeChanged(m_singleScreenMode);
    }
}

void ApplicationStateModel::setCountDownTimerState(bool state) {
    if (m_countDownTimerState != state) {
        m_countDownTimerState = state;
        auto settings = GlobalUtilities::applicationSettings("capture_button");
        settings->setValue("countdown_timer_state", m_countDownTimerState);
        emit countDownTimerStateChanged(m_countDownTimerState);
    }
}

void ApplicationStateModel::setSendToStageMode(bool autoMode) {
    if (m_sendToStageMode != autoMode) {
        m_sendToStageMode = autoMode;
        emit sendToStageModeChanged(m_sendToStageMode);
    }
}

void ApplicationStateModel::setSendToStageModeCapture(bool sendToStageModeCapture) {
    if (m_sendToStageModeCapture != sendToStageModeCapture) {
        m_sendToStageModeCapture = sendToStageModeCapture;
    }
}

void ApplicationStateModel::setPicInPicMode(bool picInPicMode) {
    if (m_picInPicMode != picInPicMode) {
        m_picInPicMode = picInPicMode;
        auto settings = GlobalUtilities::applicationSettings("capture_button");
        settings->setValue("picture_in_picture", m_picInPicMode);
        emit picInPicModeChanged(m_picInPicMode);
    }
}

void ApplicationStateModel::setInkPixMap(QPixmap inkPixMap) {
    m_inkPixmap = inkPixMap;
    emit inkPixMapChanged(m_inkPixmap);
}

void ApplicationStateModel::setColorCalibrationStatus(model::ApplicationStateModel::ColorCalibrationStatus colorCalibrationStatus) {
    if (m_colorCalibrationStatus != colorCalibrationStatus) {
        m_colorCalibrationStatus = colorCalibrationStatus;

        qInfo() << this << "Color calibration status changed to" << m_colorCalibrationStatus;

        emit colorCalibrationStatusChanged(m_colorCalibrationStatus);
    }
}

void ApplicationStateModel::setInkPenWidthScale(double inkPenWidthScale) {
    m_inkPenWidthScale = inkPenWidthScale;
}

} // namespace model
} // namespace capture
