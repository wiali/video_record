#pragma once
#ifndef COLORCORRECTIONTOGGLEBUTTON_H
#define COLORCORRECTIONTOGGLEBUTTON_H

#include <QWidget>

#include <right_menu_button.h>

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class ColorCorrectionToggleButton : public RightMenuButton
{
    Q_OBJECT
public:
    explicit ColorCorrectionToggleButton(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:
    void onButtonClicked();
    void updateState();
    void onAutoFixChanged(bool autoFix);

private:
    QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace monitor
} // namespace capture

#endif // COLORCORRECTIONTOGGLEBUTTON_H
