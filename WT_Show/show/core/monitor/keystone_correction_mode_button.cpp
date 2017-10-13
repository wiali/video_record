#include "keystone_correction_mode_button.h"

namespace capture {
namespace monitor {

KeystoneCorrectionModeButton::KeystoneCorrectionModeButton(QWidget *parent)
    : RightMenuButton(parent) {
    connect(this, &RightMenuButton::clicked, this, &KeystoneCorrectionModeButton::onButtonClicked);
}

void KeystoneCorrectionModeButton::setModel(QSharedPointer<model::ApplicationStateModel> model) {
    m_model = model;
    auto liveCapture = m_model->liveCapture();

    connect(liveCapture.data(), &model::LiveCaptureModel::keystoneCorrectionModeChanged, this, &KeystoneCorrectionModeButton::onKeystoneCorrectionModeChanged);
    onKeystoneCorrectionModeChanged(liveCapture->keystoneCorrectionMode());

    connect(liveCapture.data(), &model::LiveCaptureModel::captureStateChanged, this, &KeystoneCorrectionModeButton::updateDisabledState);
    connect(liveCapture.data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &KeystoneCorrectionModeButton::updateDisabledState);
    connect(liveCapture.data(), &model::LiveCaptureModel::selectedVideoStreamSourcesChanged, this, &KeystoneCorrectionModeButton::updateDisabledState);

    updateDisabledState();
}

void KeystoneCorrectionModeButton::onButtonClicked() {
    auto liveCapture = m_model->liveCapture();

    static QMap<model::LiveCaptureModel::KeystoneCorrectionMode, model::LiveCaptureModel::KeystoneCorrectionMode> transitionMap {
        { model::LiveCaptureModel::KeystoneCorrectionMode::NoKeystoneCorrection, model::LiveCaptureModel::KeystoneCorrectionMode::NonCroppedKeystoneCorrection },
        { model::LiveCaptureModel::KeystoneCorrectionMode::NonCroppedKeystoneCorrection, model::LiveCaptureModel::KeystoneCorrectionMode::CroppedKeystoneCorrection },
        { model::LiveCaptureModel::KeystoneCorrectionMode::CroppedKeystoneCorrection, model::LiveCaptureModel::KeystoneCorrectionMode::NoKeystoneCorrection }
    };

    liveCapture->setKeystoneCorrectionMode(transitionMap[liveCapture->keystoneCorrectionMode()]);
}

void KeystoneCorrectionModeButton::updateDisabledState() {
    auto liveCapture = m_model->liveCapture();

    bool isEnabled = liveCapture->selectedVideoStreamSources().contains(common::VideoSourceInfo::SproutCamera()) &&
                     liveCapture->videoStreamState() == model::LiveCaptureModel::VideoStreamState::Running &&
                     liveCapture->captureState() == model::LiveCaptureModel::CaptureState::NotCapturing;

    setEnabled(isEnabled);
}

void KeystoneCorrectionModeButton::onKeystoneCorrectionModeChanged(model::LiveCaptureModel::KeystoneCorrectionMode mode) {
    switch(mode) {
    case model::LiveCaptureModel::KeystoneCorrectionMode::NoKeystoneCorrection:
        setText(tr("No keystone correction"));
        setIconName("icon-no-keystone-correction");
        break;
    case model::LiveCaptureModel::KeystoneCorrectionMode::NonCroppedKeystoneCorrection:
        setText(tr("Uncropped keystone correction"));
        setIconName("icon-uncropped-keystone-correction");
        break;
    case model::LiveCaptureModel::KeystoneCorrectionMode::CroppedKeystoneCorrection:
        setText(tr("Cropped keystone correction"));
        setIconName("icon-cropped-keystone-correction");
        break;
    }
}

} // namespace monitor
} // namespace capture
