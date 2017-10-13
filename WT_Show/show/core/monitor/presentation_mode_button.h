#pragma once
#ifndef PRESENTATION_MODE_BUTTON_H
#define PRESENTATION_MODE_BUTTON_H

#include <QWidget>

#include <right_menu_button.h>
#include <user_event_handler.h>

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class PresentationModeButton : public RightMenuButton
{
    Q_OBJECT

public:
    explicit PresentationModeButton(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onButtonClicked();
    void onPresentationModeChanged(bool presentationMode);
    void updatePresentButtonState();

private:
    QSharedPointer<model::ApplicationStateModel> m_model;
    QScopedPointer<user_event_handler::EventHandler> m_eventHandler;
};

} // namespace monitor
} // namespace capture

#endif // PRESENTATION_MODE_BUTTON_H
