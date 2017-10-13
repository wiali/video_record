#pragma once
#ifndef NO_CALIBRATION_DATA_FORM_H
#define NO_CALIBRATION_DATA_FORM_H

#include <QWidget>
#include <QTimer>

#include "model/application_state_model.h"

namespace Ui {
class NoCalibrationDataForm;
}

namespace capture {
namespace monitor {

class NoCalibrationDataForm : public QWidget
{
    Q_OBJECT

public:
    explicit NoCalibrationDataForm(QWidget *parent = 0);
    ~NoCalibrationDataForm();

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onRetryTimeout();
    void onOpenControlPanelClicked();
    void onVideoStreamStateChanged(model::LiveCaptureModel::VideoStreamState state);

private:
    Ui::NoCalibrationDataForm *ui;

    QSharedPointer<model::ApplicationStateModel> m_model;
    QTimer m_retryTimer;
};

} // namespace monitor
} // namespace capture

#endif // NO_CALIBRATION_DATA_FORM_H
