#pragma once
#ifndef CAMERA_IN_USE_FORM_H
#define CAMERA_IN_USE_FORM_H

#include <QWidget>

#include "model/application_state_model.h"

namespace Ui {
class CameraInUseForm;
}

namespace capture {
namespace monitor {

class CameraInUseForm : public QWidget
{
    Q_OBJECT

public:
    explicit CameraInUseForm(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onRetryButtonReleased();

private:
    QScopedPointer<Ui::CameraInUseForm> const ui;

    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // CAMERA_IN_USE_FORM_H
