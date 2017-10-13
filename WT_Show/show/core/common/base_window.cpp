#include "base_window.h"

#include <QGuiApplication>
#include <QScreen>

#include <stage_item_form.h>

#include "utilities.h"
#include "global_utilities.h"
#include "ink_layer_widget.h"
#include "common/history_manager.h"

namespace capture {
namespace common {

BaseWindow::BaseWindow(QSharedPointer<model::ApplicationStateModel> model, QWidget* parent)
    : SharedMainWindow(parent)
    , m_model(model)
    , m_eventHandler(new user_event_handler::EventHandler)
{
    auto app = qobject_cast<QGuiApplication*>(QGuiApplication::instance());

    connect(m_model.data(), &model::ApplicationStateModel::editModeChanged, this, &BaseWindow::onEditModeChanged);
    connect(app, &QGuiApplication::primaryScreenChanged, this, &BaseWindow::onScreensChanged);

    connect(m_eventHandler.data(), &user_event_handler::EventHandler::displayCountChanged, this, &BaseWindow::onDisplayCountChanged);
}

BaseWindow::~BaseWindow()
{
    if (m_inkLayerWidget)
    {
        m_inkLayerWidget->deleteLater();
        m_inkLayerWidget.clear();
    }
}

void BaseWindow::init(StageViewer* stageViewer, bool hasInkLayer)
{
    m_stageViewer = stageViewer;

    if (m_stageViewer && hasInkLayer)
    {
        m_inkLayerWidget = new InkLayerWidget(m_model, m_stageViewer);
    }

    onScreensChanged();

    if (m_stageViewer) {
        connect(m_stageViewer, &StageViewer::initialized, this, &BaseWindow::onStageViewerInitialized, Qt::QueuedConnection);
    }

    connect(m_model.data(), &model::ApplicationStateModel::selectedProjectChanged, this, &BaseWindow::onSelectedProjectChanged);
    onSelectedProjectChanged(m_model->selectedProject());

    connect(m_model.data(), &model::ApplicationStateModel::inkWidgetChanged, this, &BaseWindow::onInkWidgetChanged);
}

QSharedPointer<model::ApplicationStateModel> BaseWindow::model()
{
    return m_model;
}

QScreen *BaseWindow::monitorScreen()
{
    return m_monitorScreen.data();
}

QScreen* BaseWindow::matScreen()
{
    return m_matScreen.data();
}

QScreen* BaseWindow::presentScreen()
{
    return m_presentScreen.data();
}

void BaseWindow::onScreensChanged() {
    onDisplayCountChanged(m_eventHandler->displayCount());
}

void BaseWindow::onDisplayCountChanged(int screenCount)
{
    Q_UNUSED(screenCount)

    m_monitorScreen = GlobalUtilities::findScreen(GlobalUtilities::MonitorScreen);
    m_matScreen = GlobalUtilities::findScreen(GlobalUtilities::MatScreen);
    m_presentScreen = GlobalUtilities::findScreen(GlobalUtilities::PresentScreen);

    if (m_ownScreenConnection) {
        disconnect(m_ownScreenConnection);
    }

    auto screen = findOwnScreen();
    if (screen)
    {
        m_ownScreenConnection = connect(screen, &QScreen::geometryChanged, this, &BaseWindow::onScreenGeometryChanged);
        onScreenGeometryChanged(screen->geometry());
    }
    else
    {
        m_ownScreenConnection = QMetaObject::Connection();
    }
}

void BaseWindow::onSelectedProjectChanged(QSharedPointer<StageProject> selectedProject)
{
    if ((m_stageViewer && !m_stageViewer->isInitialized()) ||
        m_model->mode() != model::ApplicationStateModel::Mode::Preview ||
        m_selectedProject == selectedProject)
        return;

    m_selectedProject = selectedProject;

    applySelectedProject(selectedProject);

    if (m_stageViewer && m_inkLayerWidget)
        m_inkLayerWidget->setStageRenderer(m_stageViewer->renderer());
}

void BaseWindow::onStageItemViewportChanged()
{
    auto selectedProject = m_model->selectedProject();

    if (!selectedProject.isNull())
    {
        auto metaData = selectedProject->items().first()->metadata();

        QRectF bbox(metaData->geometry());
        QSizeF stageSize(selectedProject->size());
        QRectF vp;

        vp.setLeft(-bbox.left() / bbox.width());
        vp.setTop(-bbox.top() / bbox.height());
        vp.setWidth(stageSize.width() / bbox.width());
        vp.setHeight(stageSize.height() / bbox.height());

        m_model->postCapture()->setViewport(vp);
    }
}

void BaseWindow::onStageViewerInitialized()
{
    if (m_stageViewer && m_stageViewer->isVisible() && m_model->selectedProject())
    {
        onSelectedProjectChanged(m_model->selectedProject());
    }
}

void BaseWindow::onSingleInstanceReceivedArguments(const QStringList& arguments)
{
    Q_UNUSED(arguments)
    common::Utilities::bringToTopmost(winId());
}

void BaseWindow::onScreenGeometryChanged(const QRect& geometry)  {
    Q_UNUSED(geometry)
}

StageViewer::Options BaseWindow::stageViewerOptions()
{
    return StageViewer::ItemRestrictedToViewport | StageViewer::SizeFixedToItemAspectRatio;
}

void BaseWindow::resizeEvent(QResizeEvent *event)
{
    onStageItemViewportChanged();
    SharedMainWindow::resizeEvent(event);
}

void BaseWindow::onInkWidgetChanged()
{
    if (m_inkLayerWidget)
    {
        m_inkLayerWidget->updateInkDisplay();
    }
}

void BaseWindow::onEditModeChanged(model::ApplicationStateModel::EditMenuMode mode)
{
    Q_UNUSED(mode)
//    if (m_inkLayerWidget)
//        m_inkLayerWidget->setVisible(mode!= model::ApplicationStateModel::EditMenuMode::SubMenuOpen);
}

void BaseWindow::applySelectedProject(QSharedPointer<StageProject> selectedProject)
{
    for (auto connection : m_stageProjectConnections)
    {
        disconnect(connection);
    }

    m_stageProjectConnections.clear();

    if (m_stageViewer)
    {
        if (selectedProject.isNull())
        {
            qInfo() << this << "No project selected, resetting stage viewer";

            m_stageViewer->setStage(QSharedPointer<StageProject>(), stageViewerOptions());
        }
        else
        {
            if (m_inkLayerWidget)
                m_inkLayerWidget->setInkData(selectedProject->inkData());

            auto metaData = selectedProject->items().first()->metadata().data();

            m_stageViewer->setStage(selectedProject, stageViewerOptions());

            m_stageProjectConnections << connect(metaData, &StageItemMetadata::geometryChanged, this, &BaseWindow::onStageItemViewportChanged);
            m_stageProjectConnections << connect(metaData, &StageItemMetadata::viewportChanged, this, &BaseWindow::onStageItemViewportChanged);
            m_stageProjectConnections << connect(metaData, &EditableItemMetadata::changed, this, &BaseWindow::onStageItemViewportChanged);
            onStageItemViewportChanged();

            if (m_inkLayerWidget)
            {
                common::Utilities::getHistoryManager()->onInkHistoryChanged(true);
            }
        }
    }
}

} // namespace common
} // namespace capture
