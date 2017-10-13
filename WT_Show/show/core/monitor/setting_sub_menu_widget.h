#pragma once
#ifndef SETTING_SUB_MENU_WIDGET_H
#define SETTING_SUB_MENU_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include "MenuBase.h"
#include "model/application_state_model.h"
#include "toggle_button.h"

namespace Ui {
class SettingSubMenuWidget;
}

namespace capture {
namespace monitor {

class SettingSubMenuWidget : public MenuBase
{
    Q_OBJECT

public:
    explicit SettingSubMenuWidget(QWidget *parent = 0);
    void setModel(QSharedPointer<model::ApplicationStateModel> model);

protected:
    void showEvent(QShowEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    int xOffsetOfArrow() const override;
    int yOffsetOfArrow() const override;
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void timeToClose();
    void setupConnectionButton();
    void onTargetClicked(bool checked);
    void onColorCalibrationButtonClicked();
    void onKeystoneCalibrationButtonClicked();
    void updateButtonsEnabledState();

private:
    QScopedPointer<Ui::SettingSubMenuWidget> ui;
    QTimer *m_timer;
    bool lostFocusByChildren;

    QSharedPointer<model::ApplicationStateModel> m_model;
    ToggleButton* m_countDownTimerButton;
    ToggleButton* m_picInPicButton;
};

} // namespace monitor
} // namespace capture

#endif // SETTING_SUB_MENU_WIDGET_H
