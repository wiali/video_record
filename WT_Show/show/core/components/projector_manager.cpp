#include "projector_manager.h"

#include "common/utilities.h"
#include "event/start_video_streaming_event.h"
#include "event/change_mat_mode_event.h"

namespace capture {
namespace components {

ProjectorManager::ProjectorManager(QSharedPointer<model::ApplicationStateModel> model, QObject *parent)
    : QObject(parent)
    , m_projector(new proapi::projector::Projector)
    , m_model(model)
    , m_waitingForStandby(false)
    , m_waitingForOn(false)
    , m_projectorState(true)
{
    m_originalProjectorState = m_projector->state();

    connect(m_projector.data(), &proapi::projector::Projector::stateStandby, this,
            &ProjectorManager::onProjectorStandby);
    connect(m_projector.data(), &proapi::projector::Projector::stateOn, this,
            &ProjectorManager::onProjectorOn);
    connect(m_projector.data(), &proapi::projector::Projector::stateHardwareFault, this,
            &ProjectorManager::onProjectorFailed);
    connect(m_projector.data(), &proapi::projector::Projector::stateOvertemp, this,
            &ProjectorManager::onProjectorFailed);

    connect(m_model.data(), &model::ApplicationStateModel::matModeStateChanged, this,
            &ProjectorManager::updateProjectorState);
    connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this,
            &ProjectorManager::updateProjectorState);
    connect(m_model->liveCapture().data(), &model::LiveCaptureModel::selectedVideoStreamSourcesChanged, this,
            &ProjectorManager::updateProjectorState);

    updateProjectorState();
}

ProjectorManager::~ProjectorManager() {
    qInfo() << this << "Reverting projector to original state" << m_originalProjectorState;

    if (m_originalProjectorState == "standby") {
        m_projector->off();
    } else {
        turnProjectorOn();
    }
}

void ProjectorManager::onProjectorFailed() {
    m_waitingForOn = false;
    m_waitingForStandby = false;
}

void ProjectorManager::updateProjectorState() {
    m_waitingForStandby = false;
    const auto isInPreviewMode = m_model->mode() == model::ApplicationStateModel::Mode::Preview;

    switch (m_model->matModeState()) {
    case model::ApplicationStateModel::MatModeState::TransitioningToDesktop:
    case model::ApplicationStateModel::MatModeState::TransitioningToLampOn:
    case model::ApplicationStateModel::MatModeState::TransitioningToLampOff:
    case model::ApplicationStateModel::MatModeState::TransitioningToFlash:
        // No action
        break;
    case model::ApplicationStateModel::MatModeState::TransitioningToNone:
    case model::ApplicationStateModel::MatModeState::None:
    case model::ApplicationStateModel::MatModeState::TransitioningToReprojection:
    case model::ApplicationStateModel::MatModeState::Reprojection:
    case model::ApplicationStateModel::MatModeState::Desktop:
        turnProjectorOn();
        break;
    case model::ApplicationStateModel::MatModeState::LampOn: {
        turnProjectorOn();
        if (!isInPreviewMode) {
            m_projector->grayscale();
        }
        break;
    }
    case model::ApplicationStateModel::MatModeState::Flash: {
        turnProjectorOn();
        // SPROUTSW-4687 - Keep consistency with Hurley
        if (m_model->liveCapture()->selectedVideoStreamSources().count() == 1) {
            m_projector->flash(false);
        }
        break;
    }
    case model::ApplicationStateModel::MatModeState::LampOff: {
        if (isInPreviewMode) {
            m_waitingForStandby = true;
            m_projector->off();
        } else {
            if(m_projectorState)
                turnProjectorOn();
        }
        break;
    }
    default:
        Q_UNREACHABLE();
    }
}

void ProjectorManager::onProjectorStandby() {
    if (!m_model->applicationSuspended() && !m_waitingForStandby) {
        if (m_model->matModeState() != model::ApplicationStateModel::MatModeState::LampOff ||
            m_model->matModeState() != model::ApplicationStateModel::MatModeState::TransitioningToLampOff) {
            m_projectorState = false;
            auto event = new event::ChangeMatModeEvent(event::ChangeMatModeEvent::LampOff);
            event->dispatch();
        }
    }

    m_waitingForStandby = false;
}

void ProjectorManager::onProjectorOn() {
    if (!m_model->applicationSuspended() && !m_waitingForOn) {
        if (m_model->matModeState() == model::ApplicationStateModel::MatModeState::LampOff) {
            m_projectorState = true;
            auto event = new event::ChangeMatModeEvent(event::ChangeMatModeEvent::Desktop);
            event->dispatch();
        }
    }

    m_waitingForOn = false;
}

void ProjectorManager::turnProjectorOn() {
    QString state = m_projector->state();
    if (state != "on") {
        qInfo() << "Projector state is" << state << "turning it on";
        m_waitingForOn = true;
        m_projector->on();
    } else {
        qInfo() << "Projector state is already turned on (" << state << "), nothing to do.";
    }
}

} // namespace components
} // namespace capture
