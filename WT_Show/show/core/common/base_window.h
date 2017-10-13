#pragma once
#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <stage_viewer.h>
#include <QPointer>
#include <QScopedPointer>
#include <QScreen>
#include <QSharedPointer>
#include <user_event_handler.h>

#include "ink_layer_widget.h"
#include "model/application_state_model.h"
#include "shared_main_window.h"
#include "stage_viewer.h"

namespace capture {
namespace common {

class BaseWindow : public SharedMainWindow {
    Q_OBJECT

public:
    explicit BaseWindow(QSharedPointer<model::ApplicationStateModel> model, QWidget* parent = 0);
    virtual ~BaseWindow();

public slots:
    virtual void onSingleInstanceReceivedArguments(const QStringList& arguments);

protected:
    void init(StageViewer* stageViewer, bool hasInkLayer=true);
    virtual QScreen* findOwnScreen() = 0;
    virtual void onScreenGeometryChanged(const QRect& geometry);
    virtual StageViewer::Options stageViewerOptions();
    virtual void resizeEvent(QResizeEvent *event) override;

    QSharedPointer<model::ApplicationStateModel> model();
    QScreen* monitorScreen();
    QScreen* matScreen();
    QScreen* presentScreen();
    virtual void applySelectedProject(QSharedPointer<StageProject> selectedProject);

    QSharedPointer<model::ApplicationStateModel> m_model;
    QPointer<InkLayerWidget> m_inkLayerWidget;
    QSharedPointer<StageProject> m_selectedProject;
    QVector<QMetaObject::Connection> m_stageProjectConnections;

protected slots:
    void onStageViewerInitialized();
    void onSelectedProjectChanged(QSharedPointer<StageProject> selectedProject);
    virtual void onStageItemViewportChanged();
    void onScreensChanged();
    void onInkWidgetChanged();
    void onEditModeChanged(model::ApplicationStateModel::EditMenuMode mode);
    virtual void onDisplayCountChanged(int screenCount);

private:
    void updateScreen();

private:
    QPointer<QScreen> m_monitorScreen;
    QPointer<QScreen> m_matScreen;
    QPointer<QScreen> m_presentScreen;
    QMetaObject::Connection m_ownScreenConnection;
    StageViewer* m_stageViewer;
    QScopedPointer<user_event_handler::EventHandler> m_eventHandler;
};

} // namespace common
} // namespace capture

#endif  // BASEWINDOW_H
