#pragma once
#ifndef KEYSTONE_CORRECTION_MODE_BUTTON_H
#define KEYSTONE_CORRECTION_MODE_BUTTON_H

#include <QWidget>

#include <right_menu_button.h>

#include "model/application_state_model.h"

namespace capture {
namespace monitor {

class KeystoneCorrectionModeButton : public RightMenuButton
{
    Q_OBJECT
public:
    explicit KeystoneCorrectionModeButton(QWidget *parent = nullptr);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private:
    QSharedPointer<model::ApplicationStateModel> m_model;

    void updateTextAndIcon();

private slots:

    void updateDisabledState();
    void onKeystoneCorrectionModeChanged(model::LiveCaptureModel::KeystoneCorrectionMode mode);
    void onButtonClicked();
};

} // namespace monitor
} // namespace capture

#endif // KEYSTONE_CORRECTION_MODE_BUTTON_H
