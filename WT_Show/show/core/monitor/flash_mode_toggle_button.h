#pragma once
#ifndef FLASHMODETOGGLEBUTTON_H
#define FLASHMODETOGGLEBUTTON_H

#include <QWidget>

#include <right_menu_button.h>

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class FlashModeToggleButton : public RightMenuButton
{
    Q_OBJECT

public:
    explicit FlashModeToggleButton(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private:
    QSharedPointer<model::ApplicationStateModel> m_model;

    void updateTextAndIcon();

private slots:

    void updateDisabledState();
    void onFlashModeChanged(bool flashMode);
    void onButtonClicked();
};

} // namespace monitor
} // namespace capture


#endif // FLASHMODETOGGLEBUTTON_H
