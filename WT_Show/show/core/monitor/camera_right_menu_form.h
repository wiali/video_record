#pragma once
#ifndef CAMERA_RIGHT_MENU_FORM_H
#define CAMERA_RIGHT_MENU_FORM_H

#include <shared_right_menu_form.h>
#include <QScopedPointer>

#include "model/application_state_model.h"
#include "setting_sub_menu_widget.h"

namespace Ui {
class CameraRightMenuForm;
}

namespace capture {
namespace monitor {

class CameraRightMenuForm : public SharedRightMenuForm
{
    Q_OBJECT

public:
    explicit CameraRightMenuForm(QWidget *parent = 0);

    void setModel(QSharedPointer<model::ApplicationStateModel> model);
    void setEditTargetWidget(QPointer<QWidget> targetWidget);
    void setupAnimation();

private slots:
    void onApplicationModeChanged(capture::model::ApplicationStateModel::Mode mode);
    void onSelectedProjectChanged();
    void onEditModeChanged(bool checked);
    void onEditButtonChanged(bool checked);

    virtual void on_button_roller_clicked(bool checked) override;
    void on_editButton_clicked(bool checked);

    void updateEditButtonState();
    void onMenuClosedChanged(bool isClosed);

    void on_undoWidget_clicked();
    void on_redoWidget_clicked();

	//It is used to notify stageViewer size changed to crop, if other module need use it, please add comments
    void onStageViewerSizeChanged(QSize size);

signals:
    void positionChanged();

protected:
    QSharedPointer<model::ApplicationStateModel> m_model;

private:
    QScopedPointer<Ui::CameraRightMenuForm> ui;
    bool m_isCaptionOn;
    QSharedPointer<SettingSubMenuWidget> m_settingSubMenuWidget;
    QMetaObject::Connection m_inkAddConnection;
    QMetaObject::Connection m_inkRemoveConnection;
};

} // namespace monitor
} // namespace capture

#endif // CAMERA_RIGHT_MENU_FORM_H
