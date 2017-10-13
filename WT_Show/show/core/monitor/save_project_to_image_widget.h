#pragma once
#ifndef SAVE_PROJECT_TO_IMAGE_WIDGET_H
#define SAVE_PROJECT_TO_IMAGE_WIDGET_H

#include <QWidget>

#include <right_menu_button.h>

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class SaveProjectToImageWidget : public RightMenuButton
{
    Q_OBJECT

public:
    explicit SaveProjectToImageWidget(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onButtonClicked();

private:
    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // SAVE_PROJECT_TO_IMAGE_WIDGET_H
