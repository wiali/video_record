#include "accept_keystone_calibration_button.h"

namespace capture {
namespace monitor {

AcceptKeystoneCalibrationButton::AcceptKeystoneCalibrationButton(QWidget *parent)
    : RightMenuButton(parent) {
    setIconName("icon-accept-calibration");
    setText(tr("Accept calibration"));

    connect(this, &RightMenuButton::clicked, this, &AcceptKeystoneCalibrationButton::onButtonClicked);
}

void AcceptKeystoneCalibrationButton::onButtonClicked() {
    // Temporary until exposed via calibration - just write values to configuration file

    auto settings = GlobalUtilities::applicationSettings("sprout_camera_calibration");
    const auto keystoneCalibration = m_model->keystoneCalibration();

    settings->setValue("cropped_top_left", keystoneCalibration->topLeft());
    settings->setValue("cropped_top_right", keystoneCalibration->topRight());
    settings->setValue("cropped_bottom_left", keystoneCalibration->bottomLeft());
    settings->setValue("cropped_bottom_right", keystoneCalibration->bottomRight());

    m_model->liveCapture()->setKeystoneCorrectionMode(m_model->keystoneCalibration()->preCalibrationCorrectionMode());
    m_model->setMode(model::ApplicationStateModel::Mode::LiveCapture);
}

void AcceptKeystoneCalibrationButton::setModel(QSharedPointer<model::ApplicationStateModel> model) {
    m_model = model;
}

} // namespace monitor
} // namespace capture
