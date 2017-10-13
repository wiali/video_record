#ifndef ACCEPT_KEYSTONE_CALIBRATION_BUTTON_H
#define ACCEPT_KEYSTONE_CALIBRATION_BUTTON_H

#include <right_menu_button.h>
#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class AcceptKeystoneCalibrationButton : public RightMenuButton
{
    Q_OBJECT
public:
    explicit AcceptKeystoneCalibrationButton(QWidget *parent = nullptr);
    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onButtonClicked();
private:
    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // ACCEPT_KEYSTONE_CALIBRATION_BUTTON_H
