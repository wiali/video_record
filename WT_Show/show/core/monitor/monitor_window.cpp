#include "monitor_window.h"
#include "ui_monitor_window.h"

#include <QGuiApplication>
#include <QScreen>
#include <QClipboard>
#include <QStyle>
#include <QDesktopWidget>

#include <stage_item_form.h>
#include <global_utilities.h>

#include "styled_message_box.h"

#include "event/stop_video_streaming_event.h"
#include "event/change_application_state_event.h"
#include "event/clean_event.h"
#include "common/utilities.h"
#include "monitor/camera_right_menu_form.h"
#include "monitor/export_progress_dialog.h"

namespace capture {
namespace monitor {

MonitorWindow::MonitorWindow(QSharedPointer<model::ApplicationStateModel> model,
                             QSharedPointer<components::LiveVideoStreamCompositor> compositor,
                             QWidget* parent)
    : BaseWindow(model, parent)
    , ui(new Ui::MonitorWindow)
    , m_compositor(compositor)
    , m_imageFilter(new image_filters::ImageFilter)
    , m_worktoolsLauncher(new components::WorktoolsLauncher)
    , m_matModeStateMachine(new components::MatModeStateMachine)
    , m_cameraManager(new components::VideoSourceManager(model))
    , m_segmentationEngine(new components::DocumentModeProcessor)
    , m_stageProjectExporter(new components::StageProjectExporter(model->projectsExport()))
    , m_cleanEnvironment(new components::CleanEnvironment)
    , m_isMinimizing(false) 
    , m_shutterMask(nullptr) {
    // have Qt setup the UI Form baed on the .UI file for us
    ui->setupUi(this);

    // set menus and main widget
    setLeftMenu(ui->left_menu);
    setRightMenu(ui->right_menu);
    setMainView(ui->center_right_widget);

    m_matModeStateMachine->setModel(model);

    configureCameras();

    m_exportImageProcessor.reset(new components::ExportImageProcessor(model->projectsExport()));

    qRegisterMetaType<QList<QUrl>>();
    connect(m_exportImageProcessor.data(), &components::ExportImageProcessor::clipboardUrlsReady, this, &MonitorWindow::onClipboardUrlsReady);

    ui->videoWidget->setModel(model->liveCapture(), compositor);
    ui->left_menu->setModel(model);
    ui->right_menu->setModel(model);
    ui->right_menu->setEditTargetWidget(ui->stageViewer);
    ui->cameraInUseWidget->setModel(model);
    ui->noCalibrationDataForm->setModel(model);
    ui->zoomIndicator->setModel(model);
    //ui->exportForm->setModel(model->projectsExport());
    ui->clipboardExportFinishedNotificationWidget->setModel(model);
    ui->colorCalibrationForm->setModel(model);
    ui->keystoneCornersIndicatorWidget->setModel(model, compositor);

    BaseWindow::init(ui->stageViewer);

    connect(ui->right_menu, &SharedRightMenuForm::rolled, this, &MonitorWindow::onRightMenuRoll);
    connect(ui->right_menu, &SharedRightMenuForm::unrolled, this, &MonitorWindow::onRightMenuRoll);
	//used to notify crop stageViewer size changed
    connect(ui->stageViewer, &StageViewer::sizeChanged, this, &MonitorWindow::onStageViewerSizeChanged);
    connect(m_model.data(), &model::ApplicationStateModel::matModeStateChanged, this, &MonitorWindow::onMatModeStateChanged);
    onMatModeStateChanged(m_model->matModeState());

    connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this, &MonitorWindow::onApplicationModeChanged);
    onApplicationModeChanged(m_model->mode());

    connect(model->projects().data(), &model::ObservableStageProjectCollection::added, this, &MonitorWindow::onProjectCountChanged);
    connect(model->projects().data(), &model::ObservableStageProjectCollection::removed, this, &MonitorWindow::onProjectCountChanged);

    connect(model->projectsExport().data(), &model::ProjectsExportModel::stateChanged, this, &MonitorWindow::onProjectsExportStateChanged);
    connect(model->liveCapture().data(), &model::LiveCaptureModel::captureStateChanged, this, &MonitorWindow::onCaptureStateChanged);

    connect(this, &MonitorWindow::monitorWindowResized, ui->right_menu, &CameraRightMenuForm::setupAnimation);

    resizeElements(geometry().size());

    restoreWindowSettings();
    updateMainWindowLocation();

    auto captureModeEvent = new event::ChangeMatModeEvent(model->mainWindowLocation() == model::ApplicationStateModel::MainWindowLocation::None ?
                                                       event::ChangeMatModeEvent::LampOff : event::ChangeMatModeEvent::Desktop);
    captureModeEvent->dispatch();

    connect(m_stageProjectExporter.data(), &components::StageProjectExporter::exportFailed, this, &MonitorWindow::onExportFailed);
    connect(m_exportImageProcessor.data(), &components::ExportImageProcessor::exportFailed, this, &MonitorWindow::onExportFailed);

    auto settings = GlobalUtilities::applicationSettings("clean");
    if (settings->value("clean_dmp", false).toBool()) {
        auto cleanEvent = new event::CleanEvent();
        cleanEvent->dispatch();
    }

    m_shutterMask = new QWidget(this);
    m_shutterMask->setStyleSheet("background-color: black");
    m_shutterMask->hide();
}

StageViewer* MonitorWindow::stageViewer() const { return ui->stageViewer; }
common::InkLayerWidget* MonitorWindow::inkLayerWidget() { return m_inkLayerWidget.data(); }

MonitorWindow::~MonitorWindow() {
    QClipboard* clipboard = QApplication::clipboard();
    if(clipboard->ownsClipboard()) {
        qInfo() << this << "The clipboard has capture data, clean it!";
        clipboard->clear();
    }

    m_matModeStateMachine->abort();
    m_frameCapture->abort();

    auto stopStreamsEvent = new event::StopVideoStreamingEvent();
    stopStreamsEvent->dispatch();

    delete ui;
}

void MonitorWindow::onProjectsExportStateChanged(model::ProjectsExportModel::State state)
{
    // Clipboard UI notification is handled by notification
    auto projectsExport = model()->projectsExport();

    if (state == model::ProjectsExportModel::State::PrepairingToExport &&
        projectsExport->format() != model::ProjectsExportModel::Format::Clipboard) {
        if(!common::Utilities::isStageWorkToolInstalled() &&
           projectsExport->format() == model::ProjectsExportModel::Format::Stage) {
            return;
        }
        ExportProgressDialog dialog(this);

        // Make sure that dialog is centered
        auto dialogCenter = dialog.mapToGlobal(dialog.rect().center());
        auto parentWindowCenter = mapToGlobal(rect().center());
        dialog.move(parentWindowCenter - dialogCenter);

        dialog.setModel(projectsExport);

        dialog.exec();
    }
}

bool MonitorWindow::restoreWindowSettings() {
    bool result = false;

    auto settings = GlobalUtilities::applicationSettings("monitor_window");
    auto geometry = settings->value("geometry");
    auto state = settings->value("state");

    result = !geometry.isNull();

    if (result) {
        restoreGeometry(geometry.toByteArray());
    } else {
        if (auto verticalScreen = GlobalUtilities::findScreen(GlobalUtilities::MonitorScreen)) {
            qDebug() << this << "Available vertical screen geometry" << verticalScreen->availableGeometry();

            auto geometry = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(),
                                                verticalScreen->availableGeometry());

            qDebug() << this << "No settings detected, setting window position to" << geometry;
            setGeometry(geometry);
        } else {
            qWarning() << this << "Cannot find vertical screen";
        }
    }

    if (!state.isNull()) {
        restoreState(state.toByteArray());
    }

    return result;
}

void MonitorWindow::saveWindowSettings() {
    auto settings = GlobalUtilities::applicationSettings("monitor_window");
    settings->setValue("geometry", saveGeometry());
    settings->setValue("state", saveState());
}

void MonitorWindow::updateMainWindowLocation() {
    auto monitor = monitorScreen();
    auto mat = matScreen();
    auto present = presentScreen();

    if(monitor) {
        QRect geometry = monitor->geometry().intersected(this->geometry());
        float ratio = (float)geometry.width()*geometry.height() / width() / height();
        if(ratio > 0.5)
            m_model->setMainWindowLocation(model::ApplicationStateModel::MainWindowLocation::None);
    }

    if(mat) {
        QRect geometry = mat->geometry().intersected(this->geometry());
        float ratio = (float)geometry.width()*geometry.height() / width() / height();
        if(ratio > 0.5)
            m_model->setMainWindowLocation(model::ApplicationStateModel::MainWindowLocation::MonitorOnMat);
    }

    if(present) {
        QRect geometry = present->geometry().intersected(this->geometry());
        float ratio = (float)geometry.width()*geometry.height() / width() / height();
        if(ratio > 0.5)
            m_model->setMainWindowLocation(model::ApplicationStateModel::MainWindowLocation::MonitorOnExtend);
    }
}

void MonitorWindow::onClipboardUrlsReady(QList<QUrl> urls) {
    // Qt's Clipboard needs to be invoked on main UI thread so we need to do this roundtrip ...
    auto clipboard = QApplication::clipboard();

    QMimeData* mimeData = new QMimeData();
    mimeData->setUrls(urls);

    clipboard->clear();
    clipboard->setMimeData(mimeData);
}

void MonitorWindow::resizeEvent(QResizeEvent* event) {
    BaseWindow::resizeEvent(event);

    QPoint zoomIndicatorTopLeft((m_mainView->width() - ui->zoomIndicator->width()) / 2, ui->zoomIndicator->pos().y());
    ui->zoomIndicator->move(m_mainView->pos() + zoomIndicatorTopLeft);

    QPoint clipboardExportFinishedTopLeft((ui->top_widget->width() - ui->clipboardExportFinishedNotificationWidget->width()) / 2,
                                          (ui->top_widget->height() - ui->clipboardExportFinishedNotificationWidget->height()) / 2);
    ui->clipboardExportFinishedNotificationWidget->move(clipboardExportFinishedTopLeft);

    onRightMenuRoll();

    if (m_shutterMask) {
        m_shutterMask->setGeometry(ui->center_right_widget->geometry());
    }

    ui->keystoneCornersIndicatorWidget->setGeometry(ui->center_right_widget->geometry());

    emit monitorWindowResized();
    
}

void MonitorWindow::onRightMenuRoll() {
    QPoint presentationMonitorTopLeft((width() - ui->presentationMonitorNotificationWidget->width()) / 2, 30);
    ui->presentationMonitorNotificationWidget->move(presentationMonitorTopLeft);
}

void MonitorWindow::onSingleInstanceReceivedArguments(const QStringList &arguments) {
    const auto parameters = common::Utilities::parseCommandLine(arguments);
    const QString stageGUID = "133D6F29-5F4E-4600-BBB0-2B11EFEA0148";

    if(parameters.first == common::Utilities::Ok && parameters.second.launchedFrom == stageGUID) {
        if(m_model->mode() == model::ApplicationStateModel::Mode::Preview ||
                m_model->mode() == model::ApplicationStateModel::Mode::LiveCapture) {
            m_model->setSendToStageMode(true);
            m_model->liveCapture()->inkData()->clear();
            m_model->setMode(model::ApplicationStateModel::Mode::LiveCapture);
            auto captureModeEvent = new event::ChangeMatModeEvent(event::ChangeMatModeEvent::MatMode::LampOff);
            captureModeEvent->dispatch();
        }
    }

    common::Utilities::bringToTopmost(winId());
}

void MonitorWindow::onDisplayCountChanged(int screenCount) {
    BaseWindow::onDisplayCountChanged(screenCount);

    updateMainWindowLocation();
    if(m_model->mainWindowLocation() == model::ApplicationStateModel::MainWindowLocation::MonitorOnExtend) {
        QRect rect = QApplication::desktop()->availableGeometry();
        int x = rect.x() + (rect.width() - width()) / 2;
        int y = rect.y() + (rect.height() - height()) / 2;
        x = x > rect.x() ? x : rect.x();
        y = y > rect.y() ? y : rect.y();

        showNormal();
        move(x, y);
    }
}

void MonitorWindow::changeEvent(QEvent* event) {
    BaseWindow::changeEvent(event);

    if (event->type() == QEvent::WindowStateChange) {
        bool isMinimizing = windowState().testFlag(Qt::WindowMinimized);

        // When minimizing and presentation mode is on then do nothing
        // Otherwise switch to Desktop mode
        if (isMinimizing && !m_model->presentationMode()) {
            m_model->setMatModeState(model::ApplicationStateModel::MatModeState::Desktop);
            auto event = new event::ChangeMatModeEvent(event::ChangeMatModeEvent::MatMode::Desktop);
            event->dispatch();
        }

        if(m_isMinimizing && !isMinimizing) {
            auto event = new event::ChangeMatModeEvent(common::Utilities::MatModeStateToMatMode(m_model->matModeState()));
            event->dispatch();
        }

        if(m_model->liveCapture()->captureState() == model::LiveCaptureModel::CaptureState::NotCapturing) {
            auto event = new event::ChangeApplicationStateEvent(isMinimizing);
            event->dispatch();
        }

        m_isMinimizing = isMinimizing;
    }
}

void MonitorWindow::onScreenGeometryChanged(const QRect& geometry) {
    int x = (geometry.width() - frameSize().width()) / 2;
    int y = (geometry.height() - frameSize().height()) / 2;
    y = y < 40 ? 0 : y;

    move(geometry.x() + x, geometry.y() + y);
}

void MonitorWindow::configureCameras() {
    auto outputSinkConfig = m_compositor->videoPipeline()->outputSinkConfiguration();

    // Don't stream to DirectShow virtual camera
    outputSinkConfig.name = QString();

    // Don't include compositor
    outputSinkConfig.size = QSize();

    m_compositor->videoPipeline()->setOutputSinkConfiguration(outputSinkConfig);

    for(auto videoStreamSource : m_model->liveCapture()->videoStreamSources()) {
        switch(videoStreamSource->videoSource().type) {
        case common::VideoSourceInfo::SourceType::DownwardFacingCamera: {
            auto settings = GlobalUtilities::applicationSettings("downward_facing_camera");
            videoStreamSource->setSkipFrameCount(settings->value("skip_first_frames_count", 5).toInt());
            videoStreamSource->setFrameFreezeDetectionTimeout(settings->value("video_freeze_detection_timeout", 3000).toInt());
            break;
        }
        case common::VideoSourceInfo::SourceType::SproutCamera: {
            auto settings = GlobalUtilities::applicationSettings("sprout_camera");
            videoStreamSource->setFrameFreezeDetectionTimeout(settings->value("video_freeze_detection_timeout", 3000).toInt());
            break;
        }
        }
    }

    m_videoSourceInput = QSharedPointer<components::VideoSourceInput>::create(m_compositor, m_imageFilter, model());
    m_frameCapture.reset(new components::FrameCapture(m_compositor, model()));

    connect(m_frameCapture.data(), &components::FrameCapture::captureFailed, this, &MonitorWindow::onCaptureFailed);
}

void MonitorWindow::onCaptureFailed() {
    // SPROUT-19362
    qWarning() << this << "Capture has failed, restarting the cameras";

    auto messageBox = common::Utilities::createMessageBox();

    messageBox->setText(tr("Capture failed"));
    messageBox->setInformativeText(tr("Unable to capture frames from hardware devices, please try again."));
    messageBox->addStyledButton(tr("OK"), QMessageBox::YesRole);
    messageBox->exec();

    m_model->setMode(model::ApplicationStateModel::Mode::None);
    m_model->setMode(model::ApplicationStateModel::Mode::LiveCapture);
}

void MonitorWindow::onApplicationModeChanged(model::ApplicationStateModel::Mode mode) {
    static QMap<model::ApplicationStateModel::Mode, int> tabIndexMap = {
        { model::ApplicationStateModel::Mode::None, -1 },
        { model::ApplicationStateModel::Mode::LiveCapture, 0 },
        { model::ApplicationStateModel::Mode::Preview, 1 },
        { model::ApplicationStateModel::Mode::CameraFailedToStart, 2 },
        { model::ApplicationStateModel::Mode::NoCalibrationData, 3 },
        { model::ApplicationStateModel::Mode::NoVideoSource, 4},
        { model::ApplicationStateModel::Mode::ColorCorrectionCalibration, 5 },
        { model::ApplicationStateModel::Mode::KeystoneCorrectionCalibration, 0},
    };

    const auto tabIndex = tabIndexMap[mode];

    if (tabIndex >= 0) {
        ui->centralAreaStackedWidget->setCurrentIndex(tabIndex);

        // In case that window was minimized we want to bring it forward
        if (windowState().testFlag(Qt::WindowMinimized)) {
            setWindowState(Qt::WindowActive);
        }
    }

    ui->videoWidget->setViewportManipulationEnabled(mode != model::ApplicationStateModel::Mode::KeystoneCorrectionCalibration);
    ui->videoWidget->setViewportEnabled(mode != model::ApplicationStateModel::Mode::KeystoneCorrectionCalibration);
}

void MonitorWindow::moveEvent(QMoveEvent* event) {
    updateMainWindowLocation();
    BaseWindow::moveEvent(event);
}

void MonitorWindow::closeEvent(QCloseEvent* event) {
    common::Utilities::processCloseEvent(event, model());

    if (event->isAccepted()) {
        saveWindowSettings();

        emit closing();

        BaseWindow::closeEvent(event);
    }
}

void MonitorWindow::onMatModeStateChanged(model::ApplicationStateModel::MatModeState state) {
    Q_UNUSED(state);

    activateWindow();
    raise();
}

QScreen* MonitorWindow::findOwnScreen() {
    switch(m_model->mainWindowLocation()) {
    case model::ApplicationStateModel::MainWindowLocation::None:
        return GlobalUtilities::findScreen(GlobalUtilities::MonitorScreen);
    case model::ApplicationStateModel::MainWindowLocation::MonitorOnMat:
        return GlobalUtilities::findScreen(GlobalUtilities::MatScreen);
    case model::ApplicationStateModel::MainWindowLocation::MonitorOnExtend:
        return GlobalUtilities::findScreen(GlobalUtilities::PresentScreen);
    }

    Q_UNREACHABLE();
}

void MonitorWindow::onExportFailed(const QString& msg) {
    qInfo() << this << "Export failed with reason" << msg;

    auto messageBox = common::Utilities::createMessageBox();
    messageBox->setText(tr("Failed to export"));
    messageBox->setInformativeText(tr("Unable to export captures. Please check the disk space and try again."));
    messageBox->addStyledButton(tr("OK"), QMessageBox::YesRole);
    messageBox->exec();
}

void MonitorWindow::onProjectCountChanged() {
    if (GlobalUtilities::applicationSettings("monitor_window")->value("show_debug_info", false).toBool()) {
        const auto title = QString("Captures: %1, version: %2, %3")
                .arg(m_model->projects()->count())
                .arg(QApplication::instance()->applicationVersion())
                .arg(m_model->singleScreenMode() ? "single screen mode" : "Sprout mode");

        setWindowTitle(title);
    }
}

void MonitorWindow::onCaptureStateChanged(model::LiveCaptureModel::CaptureState captureState) {
    if (captureState == model::LiveCaptureModel::CaptureState::Capturing &&
        // SPROUTSW-4652 - don't show shutter mask if we are capturing primary desktop
        !m_model->liveCapture()->selectedVideoStreamSources().contains(common::VideoSourceInfo::PrimaryDesktop())) {
        m_shutterMask->show();

        // Make sure that shutter mask will be visible only for predefined duration
        QTimer::singleShot(GlobalUtilities::applicationSettings("monitor_window")->value("shutter_mask_duration", 100).toInt(),
                           m_shutterMask, &QWidget::hide);
    } else {
        m_shutterMask->hide();
    }
}

void MonitorWindow::onStageViewerSizeChanged(QSize size)
{
    emit stageViewerSizeChanged(size);
}

} // namespace monitor
} // namespace capture
