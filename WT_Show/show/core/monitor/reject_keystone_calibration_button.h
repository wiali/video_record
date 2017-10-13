#ifndef REJECT_KEYSTONE_CALIBRATION_BUTTON_H
#define REJECT_KEYSTONE_CALIBRATION_BUTTON_H

#include <right_menu_button.h>
#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class RejectKeystoneCalibrationButton : public RightMenuButton
{
    Q_OBJECT
public:
    explicit RejectKeystoneCalibrationButton(QWidget *parent = nullptr);
    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onButtonClicked();
private:
    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // REJECT_KEYSTONE_CALIBRATION_BUTTON_H
