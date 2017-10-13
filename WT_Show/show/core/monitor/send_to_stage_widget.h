#pragma once
#ifndef SEND_TO_STAGE_WIDGET_H
#define SEND_TO_STAGE_WIDGET_H

#include <QWidget>

#include <right_menu_button.h>

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class SendToStageWidget : public RightMenuButton
{
    Q_OBJECT

public:
    explicit SendToStageWidget(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onButtonClicked();
    void onModeChange();

private:
    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // SEND_TO_STAGE_WIDGET_H
