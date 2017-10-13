#pragma once
#ifndef PROJECT_ITEM_FORM_H
#define PROJECT_ITEM_FORM_H

#include <QWidget>

#include <atomic>

#include <stage_project.h>

#include "model/application_state_model.h"
#include "monitor/invalid_project_name_widget.h"
#include "components/project_name_validator.h"
#include "common/offscreen_renderer.h"

namespace Ui {
class ProjectItemForm;
}

namespace capture {
namespace monitor {

class ProjectItemForm : public QWidget
{
    Q_OBJECT    
    Q_PROPERTY(bool highligted READ highligted WRITE setHighlighted NOTIFY highligtedChanged)

public:
    explicit ProjectItemForm(QWidget *parent = 0);
    ~ProjectItemForm();

    void setModel(QSharedPointer<model::ApplicationStateModel> applicationStateModel, QSharedPointer<StageProject> projectModel);
    QSharedPointer<StageProject> projectModel() const;
    bool highligted() const;

public slots:

    void setHighlighted(bool highligted);

signals:

    void highligtedChanged(bool highligted);

private slots:

    void onThumbnailChanged(QImage thumbnail);
    void onSelectedChanged(bool selected);
    void onHighlightedChanged(bool selected);
    void onProjectNameChanged(QString name);
    void onCaptureStateChanged(model::LiveCaptureModel::CaptureState captureState);

    void onProjectNameTextConfirmed();
    void onInvalidCharacterEntered();
    void onInkDataChanged();

    void enableNameEdit();
    void disableNameEdit();

    void on_deleteButton_clicked();

    void onModeChanged(model::ApplicationStateModel::Mode mode);

    void onEditModeChanged(model::ApplicationStateModel::EditMenuMode editMode);
    void onImageReady(QSharedPointer<StageItem> item, QImage image);
    void updateThumbnail(bool forceUpdate);
    void updateInkData(bool withInk);
    void onUndoRedoChanged();

protected:

    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    Ui::ProjectItemForm *ui;
    QSharedPointer<StageProject> m_projectModel;
    QSharedPointer<model::ApplicationStateModel> m_applicationStateModel;
    QScopedPointer<components::ProjectNameValidator> m_projectNameValidator;
    QPixmap m_renderedImage;

    QStringList m_invalidCharacters;
    QScopedPointer<common::OffScreenRenderer> m_offscreenRenderer;
    bool m_highligted;
    std::atomic_bool mIsCollisionDialogOpen;

    QImage m_lastOffscreenImage;
    QMutex m_offscreenRendererMutex;
    QWaitCondition m_offscreenRendererWaitCondition;
    QSharedPointer<StageProject> m_currentProject;
};

} // namespace monitor
} // namespace capture

#endif // PROJECT_ITEM_FORM_H
