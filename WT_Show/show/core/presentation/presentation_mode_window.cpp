#include "presentation_mode_window.h"
#include "ui_presentation_mode_window.h"

#include <QScreen>

#include "common/utilities.h"
#include "global_utilities.h"

namespace capture {
namespace presentation {

PresentationModeWindow::PresentationModeWindow(QSharedPointer<model::ApplicationStateModel> model,
                                               QSharedPointer<components::LiveVideoStreamCompositor> compositor,
                                               QWidget *parent)
    : BaseWindow(model, parent)
    , ui(new Ui::PresentationModeWindow)
    , m_eventHandler(new user_event_handler::EventHandler) {
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::Tool);

    connect(model.data(), &model::ApplicationStateModel::modeChanged, this, &PresentationModeWindow::onApplicationModeChanged);
    connect(model.data(), &model::ApplicationStateModel::presentationModeChanged, this, &PresentationModeWindow::onPresentModeChanged);
    connect(model.data(), &model::ApplicationStateModel::mainWindowLocationChanged, this, &PresentationModeWindow::onMainWindowLocationChanged);
    connect(m_eventHandler.data(), &user_event_handler::EventHandler::displayCountChanged, this, &PresentationModeWindow::onPresentScreenGeometryChanged);
    onPresentScreenGeometryChanged();

    BaseWindow::init(nullptr);

    ui->videoWidget->setModel(model->liveCapture(), compositor);
    ui->videoWidget->setViewportManipulationEnabled(false);
    onApplicationModeChanged(model->mode());

    ui->centralwidget->setStyleSheet("background-color:black;");
}

PresentationModeWindow::~PresentationModeWindow() {}

void PresentationModeWindow::onApplicationModeChanged(model::ApplicationStateModel::Mode mode) {
    ui->centralAreaStackedWidget->setCurrentIndex(mode == model::ApplicationStateModel::Mode::Preview ? 1 : 0);
    ui->videoWidget->setViewportEnabled(mode != model::ApplicationStateModel::Mode::KeystoneCorrectionCalibration);
}

void PresentationModeWindow::onScreenGeometryChanged(const QRect &geometry) {
    Q_UNUSED(geometry);
    onPresentScreenGeometryChanged();
}

void PresentationModeWindow::onPresentScreenGeometryChanged() {
    QRect geometry = GlobalUtilities::findScreenGeometry(GlobalUtilities::PresentScreen);
    if(!geometry.isNull()) {
        setGeometry(geometry);
        onStageProjectSizeChanged();
    } else {
        this->hide();
    }
}

void PresentationModeWindow::onPresentModeChanged(bool isPresent) {
    qDebug() << "The Present mode is changing to" << isPresent;
    if (isPresent) {
        applySelectedProject(model()->selectedProject());
    }
    setVisible(isPresent);
}

void PresentationModeWindow::onMainWindowLocationChanged(model::ApplicationStateModel::MainWindowLocation location) {
    if (location == model::ApplicationStateModel::MainWindowLocation::MonitorOnExtend) {
        model()->setPresentationMode(false);
    }
}

void PresentationModeWindow::onStageItemViewportChanged() {
    onStageProjectSizeChanged();
    BaseWindow::onStageItemViewportChanged();
}

void PresentationModeWindow::onStageProjectSizeChanged() {
    if (m_model->selectedProject()) {
        QRect itemGeometry = m_model->selectedProject()->items().first()->metadata()->geometry();
        QSize projectSize = m_model->selectedProject()->size();
        QSize targetSize = itemGeometry.intersected(QRect(QPoint(0, 0), projectSize)).size();
        QSize newSize = projectSize.scaled(targetSize.scaled(this->size(), Qt::KeepAspectRatio), Qt::KeepAspectRatioByExpanding);
        QRect newRect(QPoint(0, 0), newSize);
        newRect.moveCenter(this->rect().center());
        ui->centralAreaStackedWidget->setGeometry(newRect);
        ui->stageViewerMirror->renderImage(true);
    }
}

void PresentationModeWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
    model()->setPresentationMode(false);
}

QScreen* PresentationModeWindow::findOwnScreen() {
    return GlobalUtilities::findScreen(GlobalUtilities::PresentScreen);
}

StageViewer::Options PresentationModeWindow::stageViewerOptions() {
    return StageViewer::DisableControls | BaseWindow::stageViewerOptions();
}

void PresentationModeWindow::applySelectedProject(QSharedPointer<StageProject> selectedProject) {
    BaseWindow::applySelectedProject(selectedProject);
    onStageProjectSizeChanged();

    if (selectedProject) {
        auto metaData = selectedProject->items().first()->metadata().data();
        m_stageProjectConnections << connect(selectedProject.data(), &StageProject::sizeChanged,
                                             this, &PresentationModeWindow::onStageProjectSizeChanged);
        m_stageProjectConnections << connect(metaData, &StageItemMetadata::geometryChanged,
                                             this, &PresentationModeWindow::onStageItemViewportChanged);
        m_stageProjectConnections << connect(metaData, &StageItemMetadata::viewportChanged,
                                             this, &PresentationModeWindow::onStageItemViewportChanged);
        m_stageProjectConnections << connect(metaData, &EditableItemMetadata::changed,
                                             this, &PresentationModeWindow::onStageItemViewportChanged);
        onStageItemViewportChanged();
    }
}

void PresentationModeWindow::onSingleInstanceReceivedArguments(const QStringList& arguments) {
    Q_UNUSED(arguments)

    if(model()->presentationMode()) {
        common::Utilities::bringToTopmost(winId());
    }
}

void PresentationModeWindow::setSources(StageViewer* stageViewer, common::InkLayerWidget* inkWidget)
{
    ui->stageViewerMirror->setSources(stageViewer, inkWidget);
}

} // namespace presentation
} // namespace capture

