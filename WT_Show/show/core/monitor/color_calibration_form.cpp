#include "color_calibration_form.h"
#include "ui_color_calibration_form.h"

#include "event/start_color_calibration_event.h"

namespace capture {
namespace monitor {

ColorCalibrationForm::ColorCalibrationForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorCalibrationForm)
{
    ui->setupUi(this);

    connect(ui->startCalibrationButton, &QPushButton::clicked, this, &ColorCalibrationForm::onStartCalibrationButtonClicked);
    connect(ui->returnToLiveCaptureButton, &QPushButton::clicked, this, &ColorCalibrationForm::onReturnToLiveCaptureButtonClicked);
}

void ColorCalibrationForm::setModel(QSharedPointer<model::ApplicationStateModel> model) {
    m_model = model;

    connect(model.data(), &model::ApplicationStateModel::colorCalibrationStatusChanged, this, &ColorCalibrationForm::onColorCalibrationStatusChanged);
    onColorCalibrationStatusChanged(model->colorCalibrationStatus());
}

void ColorCalibrationForm::onStartCalibrationButtonClicked() {
    auto event = new event::StartColorCalibrationEvent();
    event->dispatch();
}

void ColorCalibrationForm::onReturnToLiveCaptureButtonClicked() {
    m_model->setMode(model::ApplicationStateModel::Mode::LiveCapture);
}

void ColorCalibrationForm::onColorCalibrationStatusChanged(model::ApplicationStateModel::ColorCalibrationStatus colorCalibrationStatus) {
    switch(colorCalibrationStatus) {
    case model::ApplicationStateModel::ColorCalibrationStatus::NotCalibrating:
        ui->startCalibrationButton->show();
        ui->label->hide();
        ui->progressBar->hide();
        ui->returnToLiveCaptureButton->hide();
        return;
    case model::ApplicationStateModel::ColorCalibrationStatus::CapturingImages:
        ui->startCalibrationButton->hide();
        ui->returnToLiveCaptureButton->hide();
        ui->label->setText(tr("Capturing images ..."));
        ui->label->show();
        ui->progressBar->setValue(1);
        ui->progressBar->show();
        return;
    case model::ApplicationStateModel::ColorCalibrationStatus::PerformingCalibration:
        ui->startCalibrationButton->hide();
        ui->returnToLiveCaptureButton->hide();
        ui->label->setText(tr("Performing calibration ..."));
        ui->label->show();

        ui->progressBar->setValue(2);
        ui->progressBar->show();
        return;
    case model::ApplicationStateModel::ColorCalibrationStatus::CalibrationComplete:
        ui->startCalibrationButton->hide();
        ui->returnToLiveCaptureButton->show();
        ui->label->setText(tr("Calibration succesfull!"));
        ui->label->show();

        ui->progressBar->setValue(3);
        ui->progressBar->show();
        return;
    }

    Q_UNREACHABLE();
}

} // namespace monitor
} // namespace capture
