#include "reject_keystone_calibration_button.h"

namespace capture {
namespace monitor {

RejectKeystoneCalibrationButton::RejectKeystoneCalibrationButton(QWidget *parent)
    : RightMenuButton(parent) {
    setIconName("icon-reject-calibration");
    setText(tr("Reject calibration"));

    connect(this, &RightMenuButton::clicked, this, &RejectKeystoneCalibrationButton::onButtonClicked);
}

void RejectKeystoneCalibrationButton::onButtonClicked() {
    // Temporary until exposed via calibration - just reset to values from configuration file
    auto settings = GlobalUtilities::applicationSettings("sprout_camera_calibration");
    auto keystoneCalibration = m_model->keystoneCalibration();
    keystoneCalibration->setTopLeft(settings->value("cropped_top_left", QPoint(600, 300)).toPoint());
    keystoneCalibration->setTopRight(settings->value("cropped_top_right", QPoint(-600, 300)).toPoint());
    keystoneCalibration->setBottomLeft(settings->value("cropped_bottom_left", QPoint(200, -400)).toPoint());
    keystoneCalibration->setBottomRight(settings->value("cropped_bottom_right", QPoint(-200, -400)).toPoint());

    m_model->liveCapture()->setKeystoneCorrectionMode(m_model->keystoneCalibration()->preCalibrationCorrectionMode());
    m_model->setMode(model::ApplicationStateModel::Mode::LiveCapture);
}

void RejectKeystoneCalibrationButton::setModel(QSharedPointer<model::ApplicationStateModel> model) {
    m_model = model;
}

} // namespace monitor
} // namespace capture

