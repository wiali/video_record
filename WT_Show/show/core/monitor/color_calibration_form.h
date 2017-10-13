#pragma once
#ifndef COLOR_CALIBRATION_FORM_H
#define COLOR_CALIBRATION_FORM_H

#include <QWidget>
#include <QScopedPointer>

#include "model/application_state_model.h"

namespace Ui {
class ColorCalibrationForm;
}

namespace capture {
namespace monitor {

class ColorCalibrationForm : public QWidget
{
    Q_OBJECT

public:
    explicit ColorCalibrationForm(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:

    void onStartCalibrationButtonClicked();
    void onReturnToLiveCaptureButtonClicked();
    void onColorCalibrationStatusChanged(model::ApplicationStateModel::ColorCalibrationStatus colorCalibrationStatus);

private:
    QScopedPointer<Ui::ColorCalibrationForm> ui;
    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // COLOR_CALIBRATION_FORM_H
