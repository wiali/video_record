#include "touchmat_manager.h"

namespace capture {
namespace components {

TouchmatManager::TouchmatManager(QSharedPointer<model::ApplicationStateModel> model, QObject *parent)
    : QObject(parent), m_model(model), m_touchmat(new proapi::touchmat::Touchmat), m_originalTouchState(false) {
  auto app = qobject_cast<QGuiApplication *>(QApplication::instance());
  connect(app, &QApplication::applicationStateChanged, this,
          &TouchmatManager::onApplicationStateChanged);

  connect(m_model.data(), &model::ApplicationStateModel::matModeStateChanged, this,
          &TouchmatManager::updateTouchmatState);
  connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this,
          &TouchmatManager::updateTouchmatState);
  connect(m_model->liveCapture().data(), &model::LiveCaptureModel::selectedVideoStreamSourcesChanged, this,
          &TouchmatManager::updateTouchmatState);

  updateTouchmatState();
}

TouchmatManager::~TouchmatManager() {
  qInfo() << this << "Reverting touchmat to original state" << m_originalTouchState;

  m_touchmat->setTouchState(m_originalTouchState);
}

void TouchmatManager::updateTouchmatState() {
    switch (m_model->matModeState()) {
    case model::ApplicationStateModel::MatModeState::TransitioningToDesktop:
    case model::ApplicationStateModel::MatModeState::TransitioningToLampOn:
    case model::ApplicationStateModel::MatModeState::TransitioningToLampOff:
    case model::ApplicationStateModel::MatModeState::TransitioningToNone:
    case model::ApplicationStateModel::MatModeState::None:
    case model::ApplicationStateModel::MatModeState::TransitioningToReprojection:
    case model::ApplicationStateModel::MatModeState::Reprojection:
        // No action
        break;
    case model::ApplicationStateModel::MatModeState::Desktop:
        if(!m_touchmat->state().touch){
            m_touchmat->setTouchState(true);
        }
        break;
    case model::ApplicationStateModel::MatModeState::Flash:
    case model::ApplicationStateModel::MatModeState::LampOn:
    case model::ApplicationStateModel::MatModeState::LampOff: {
        if(m_touchmat->state().touch){
            m_touchmat->setTouchState(false);
        }
        break;
    }

    default:
        Q_UNREACHABLE();
    }
}

void TouchmatManager::onApplicationStateChanged(Qt::ApplicationState state) {
  if (state == Qt::ApplicationActive) {
    // SPROUT-18662 - when activating the application let's make sure that we set parameters
    // based on current settings

    m_originalTouchState = m_touchmat->state().touch;
  }
}

} // namespace components
} // namespace capture
