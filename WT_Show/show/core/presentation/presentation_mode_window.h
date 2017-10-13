#pragma once
#ifndef PRESENTATION_MODE_WINDOW_H
#define PRESENTATION_MODE_WINDOW_H

#include <user_event_handler.h>

#include "common/base_window.h"
#include "components/live_video_stream_compositor.h"
#include "components/video_source_input.h"

namespace Ui {
class PresentationModeWindow;
}

namespace capture {
namespace presentation {

class PresentationModeWindow : public capture::common::BaseWindow
{
    Q_OBJECT

public:
    explicit PresentationModeWindow(QSharedPointer<model::ApplicationStateModel> model,
                                    QSharedPointer<components::LiveVideoStreamCompositor> compositor,
                                    QWidget *parent = 0);
    virtual ~PresentationModeWindow();

    void setSources(StageViewer* stageViewer, capture::common::InkLayerWidget* inkWidget);

protected:
    virtual void onScreenGeometryChanged(const QRect &geometry) override;
    virtual QScreen* findOwnScreen() override;
    virtual StageViewer::Options stageViewerOptions() override;
    virtual void applySelectedProject(QSharedPointer<StageProject> selectedProject) override;

public slots:
    virtual void onSingleInstanceReceivedArguments(const QStringList& arguments) override;

protected slots:

    virtual void onStageItemViewportChanged() override;

private slots:
    void onPresentScreenGeometryChanged();
    void onApplicationModeChanged(model::ApplicationStateModel::Mode mode);
    void onPresentModeChanged(bool isPresent);
    void onMainWindowLocationChanged(model::ApplicationStateModel::MainWindowLocation location);
    void onStageProjectSizeChanged();

    void closeEvent(QCloseEvent *event);

private:
    QScopedPointer<Ui::PresentationModeWindow> ui;
    QScopedPointer<user_event_handler::EventHandler> m_eventHandler;
};

} // namespace presentation
} // namespace capture

#endif // PRESENTATION_MODE_WINDOW_H
