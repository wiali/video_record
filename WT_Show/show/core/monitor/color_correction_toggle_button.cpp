#include "color_correction_toggle_button.h"

namespace capture {
namespace monitor {

ColorCorrectionToggleButton::ColorCorrectionToggleButton(QWidget *parent)
    : RightMenuButton(parent)
{
    setText(tr("Auto Fix"));
    setIconName("icon-colorcorrection");
    setCheckable(true);

    connect(this, &RightMenuButton::clicked, this, &ColorCorrectionToggleButton::onButtonClicked);
}

void ColorCorrectionToggleButton::setModel(QSharedPointer<model::ApplicationStateModel> model)
{
    m_model = model;
    auto liveCapture = m_model->liveCapture();

    connect(m_model.data(), &model::ApplicationStateModel::matModeStateChanged, this, &ColorCorrectionToggleButton::updateState);
    connect(liveCapture.data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &ColorCorrectionToggleButton::updateState);
    connect(liveCapture.data(), &model::LiveCaptureModel::captureStateChanged, this, &ColorCorrectionToggleButton::updateState);
    updateState();

    connect(liveCapture.data(), &model::LiveCaptureModel::autoFixChanged, this, &ColorCorrectionToggleButton::onAutoFixChanged);
    onAutoFixChanged(liveCapture->autoFix());
}

void ColorCorrectionToggleButton::onButtonClicked()
{
    m_model->liveCapture()->setAutoFix(!m_model->liveCapture()->autoFix());
}

void ColorCorrectionToggleButton::onAutoFixChanged(bool autoFix)
{
    setChecked(autoFix);
}

void ColorCorrectionToggleButton::updateState()
{
    auto liveCapture = m_model->liveCapture();
    auto isEnabled = liveCapture->selectedVideoStreamSources().contains(common::VideoSourceInfo::DownwardFacingCamera());
    isEnabled &= m_model->matModeState() == model::ApplicationStateModel::MatModeState::LampOff || m_model->matModeState() == model::ApplicationStateModel::MatModeState::LampOn;
    isEnabled &= liveCapture->captureState() == model::LiveCaptureModel::CaptureState::NotCapturing;
    isEnabled &= liveCapture->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running;
    isEnabled &= !m_model->isInTransitionalMatMode();

    setEnabled(isEnabled);
}

} // namespace monitor
} // namespace capture
