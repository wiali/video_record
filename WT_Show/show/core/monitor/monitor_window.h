#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "components/mat_mode_state_machine.h"
#include "components/frame_capture.h"
#include "components/video_source_manager.h"
#include "components/document_mode_processor.h"
#include "components/worktools_launcher.h"
#include "components/export_image_processor.h"
#include "components/stage_project_exporter.h"
#include "components/clean_environment.h"
#include "components/live_video_stream_compositor.h"
#include "components/video_source_input.h"
#include "model/application_state_model.h"

#include <image_filters/imagefilter.h>
#include <video_source/videopipeline.h>

#include "common/base_window.h"
#include <QSharedPointer>
#include <QScopedPointer>
#include <QThread>

namespace Ui {
class MonitorWindow;
}

namespace capture {
namespace monitor {

class MonitorWindow : public capture::common::BaseWindow
{
    Q_OBJECT

public:
    explicit MonitorWindow(QSharedPointer<model::ApplicationStateModel> model, QSharedPointer<components::LiveVideoStreamCompositor> compositor, QWidget *parent = 0);
    virtual ~MonitorWindow();

    inline QSharedPointer<components::VideoSourceInput> videoSourceInput() { return m_videoSourceInput; }
    StageViewer* stageViewer() const;
    capture::common::InkLayerWidget* inkLayerWidget();

signals:

    void closing();
    void monitorWindowResized();
    //used to notify camera right menu stageViewer size changed
    void stageViewerSizeChanged(QSize size);

private slots:
    void onApplicationModeChanged(capture::model::ApplicationStateModel::Mode mode);
    void onClipboardUrlsReady(QList<QUrl> urlList);
    void onProjectsExportStateChanged(model::ProjectsExportModel::State state);
    void onMatModeStateChanged(capture::model::ApplicationStateModel::MatModeState state);
    void onRightMenuRoll();
    void closeEvent(QCloseEvent *event) override;
    void onExportFailed(const QString& msg);
    void onCaptureFailed();
    void onProjectCountChanged();
    void onCaptureStateChanged(capture::model::LiveCaptureModel::CaptureState captureState);
    /**
     * @brief onStageViewerSizeChanged trigger it when stageViewer size changed
     * and emit stageViewerSizeChanged(QSize size);
     * @param size
     */
    void onStageViewerSizeChanged(QSize size);

protected:
    virtual void changeEvent(QEvent* event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent* event) override;
    virtual QScreen* findOwnScreen() override;
    virtual void onScreenGeometryChanged(const QRect &geometry) override;
    virtual void onSingleInstanceReceivedArguments(const QStringList& arguments) override;
    virtual void onDisplayCountChanged(int screenCount);

private:
    void configureCameras();
    bool restoreWindowSettings();
    void saveWindowSettings();
    void updateMainWindowLocation();

    Ui::MonitorWindow *ui;

    QSharedPointer<components::LiveVideoStreamCompositor> m_compositor;
    QSharedPointer<image_filters::ImageFilter> m_imageFilter;
    QScopedPointer<components::FrameCapture> m_frameCapture;
    QScopedPointer<components::WorktoolsLauncher> m_worktoolsLauncher;
    QScopedPointer<components::CleanEnvironment> m_cleanEnvironment;

    video::source::SourcePipeline::SourcePipelineType m_lastSelectedCamera;

    QScopedPointer<components::MatModeStateMachine> m_matModeStateMachine;
    QScopedPointer<components::VideoSourceManager> m_cameraManager;
    QScopedPointer<components::DocumentModeProcessor> m_segmentationEngine;
    QScopedPointer<components::ExportImageProcessor> m_exportImageProcessor;
    QScopedPointer<components::StageProjectExporter> m_stageProjectExporter;
    bool m_isMinimizing;
    QSharedPointer<components::VideoSourceInput> m_videoSourceInput;
    QWidget* m_shutterMask;
};

} // namespace monitor
} // namespace capture

#endif // MAINWINDOW_H
