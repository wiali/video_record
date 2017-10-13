#include "flash_mode_toggle_button.h"

#include <QDebug>

namespace capture {
namespace monitor {

FlashModeToggleButton::FlashModeToggleButton(QWidget *parent)
    : RightMenuButton(parent) {
    connect(this, &RightMenuButton::clicked, this, &FlashModeToggleButton::onButtonClicked);
    setCheckable(true);
}

void FlashModeToggleButton::setModel(QSharedPointer<model::ApplicationStateModel> model) {
    m_model = model;
    auto liveCapture = m_model->liveCapture();

    connect(liveCapture.data(), &model::LiveCaptureModel::flashModeChanged, this, &FlashModeToggleButton::onFlashModeChanged);
    onFlashModeChanged(liveCapture->flashMode());

    connect(liveCapture.data(), &model::LiveCaptureModel::captureStateChanged, this, &FlashModeToggleButton::updateDisabledState);
    connect(liveCapture.data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &FlashModeToggleButton::updateDisabledState);
    connect(liveCapture.data(), &model::LiveCaptureModel::selectedVideoStreamSourcesChanged, this, &FlashModeToggleButton::updateDisabledState);
    connect(model.data(), &model::ApplicationStateModel::matModeStateChanged, this, &FlashModeToggleButton::updateDisabledState);

    updateDisabledState();
}

void FlashModeToggleButton::onButtonClicked() {
    auto liveCapture = m_model->liveCapture();

    liveCapture->setFlashMode(!liveCapture->flashMode());
}

void FlashModeToggleButton::updateDisabledState() {
    auto liveCapture = m_model->liveCapture();

    bool isEnabled = liveCapture->supportsFlashCapture() &&
                     // SPROUTSW-4687 - with multiple sources we are taking frame directly from compositor so it doesn't make sense to have Flash capture
                     liveCapture->selectedVideoStreamSources().count() == 1 &&
                     liveCapture->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running &&
                     (m_model->matModeState() == model::ApplicationStateModel::MatModeState::LampOn ||
                      m_model->matModeState() == model::ApplicationStateModel::MatModeState::LampOff) &&
                     liveCapture->captureState() == model::LiveCaptureModel::CaptureState::NotCapturing;

    setEnabled(isEnabled);
}

void FlashModeToggleButton::onFlashModeChanged(bool flashMode) {
    setChecked(flashMode);

    if (flashMode) {
        setText(tr("Flash on"));
        setIconName("icon-flashon");
    } else {
        setText(tr("Flash off"));
        setIconName("icon-flashoff");
    }
}

} // namespace monitor
} // namespace capture

