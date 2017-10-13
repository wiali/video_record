#pragma once
#ifndef CAMERA_LEFT_MENU_FORM_H
#define CAMERA_LEFT_MENU_FORM_H

#include <shared_left_menu_form.h>

#include "model/application_state_model.h"
#include "export_widget.h"

namespace Ui {
class CameraLeftMenuForm;
}

namespace capture {
namespace monitor {

class CameraLeftMenuForm : public SharedLeftMenuForm
{
    Q_OBJECT

public:
    explicit CameraLeftMenuForm(QWidget *parent = 0);
    ~CameraLeftMenuForm();

    void setModel(QSharedPointer<model::ApplicationStateModel> model);

private slots:

    void onProjectAdded(QSharedPointer<StageProject> project);
    void onProjectRemoved(QSharedPointer<StageProject> project);

    void onSelectedProjectChanged(QSharedPointer<StageProject> selectedProject);
    void onCurrentRowChanged();
    void onApplicationModeChanged(model::ApplicationStateModel::Mode mode);
    void onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void onRowsMoved(QModelIndex sourceParent,int sourceStart,int sourceEnd,QModelIndex destinationParent,int destinationRow);    

    void on_button_new_clicked();
    void on_button_export_clicked(bool checked);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateDisabledState();
    void ensureLastProjectIsVisible();
    void onEditModeChanged(model::ApplicationStateModel::EditMenuMode mode);

    Ui::CameraLeftMenuForm *ui;
    QSharedPointer<model::ApplicationStateModel> m_model;
    ExportWidget* exportWidget;
    QWidget* m_mask;
    bool m_isRemovingItem;
};

} // namespace monitor
} // namespace capture

#endif // CAMERA_LEFT_MENU_FORM_H
