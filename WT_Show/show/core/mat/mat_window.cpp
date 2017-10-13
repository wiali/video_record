#include "mat_window.h"

#include <global_utilities.h>

#include "common/utilities.h"

#include "ui_mat_window.h"
#include "common/utilities.h"
#include "ink_widget.h"

namespace capture {
namespace mat {

const QString WhiteBackgroundStylesheet = "QWidget{ background: white;}";
const QString BlackBackgroundStylesheet = "QWidget{ background: black;}";
const QString TransparentBackgroundStylesheet = "QWidget{ background: transparent;}";

static QHash<bool, QString> ViewfinderStylesheets {
    { true, "QFrame { color: #1584ff; background: transparent }" },
    { false, "QFrame { color: transparent; background: transparent }" },
};

static QHash<QString, StageItem::ImageMode> StylesheetImageModes{
    { WhiteBackgroundStylesheet, StageItem::ImageMode::White },
    { BlackBackgroundStylesheet, StageItem::ImageMode::Black },
    { TransparentBackgroundStylesheet, StageItem::ImageMode::Transparent },
};

MatWindow::MatWindow(QSharedPointer<model::ApplicationStateModel> model, QSharedPointer<components::LiveVideoStreamCompositor> compositor,  QWidget *parent)
    : BaseWindow(model, parent)
    , ui(new Ui::MatWindow)
    , m_model(model)
    , m_inkButton(new InkButton(this))
    , m_inkStart(false)
    , m_compositor(compositor)
    , m_stageProject(new StageProject())
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground, true);

    BaseWindow::init(ui->stageViewer, false);

    auto liveCapture = m_model->liveCapture();

    connect(liveCapture.data(), &model::LiveCaptureModel::viewportChanged, this, &MatWindow::onViewportChanged);
    connect(liveCapture.data(), &model::LiveCaptureModel::captureStateChanged, this, &MatWindow::onCaptureStateChanged);
    connect(liveCapture.data(), &model::LiveCaptureModel::videoStreamStateChanged, this, &MatWindow::onViewportChanged);
    connect(liveCapture.data(), &model::LiveCaptureModel::selectedVideoStreamSourcesChanged, this, &MatWindow::onSelectedVideoStreamSourcesChanged);

    connect(m_model->postCapture().data(), &model::PostCaptureModel::viewportChanged, this, &MatWindow::onViewportChanged);
    connect(m_model.data(), &model::ApplicationStateModel::modeChanged, this, &MatWindow::onModeChanged);
    connect(m_model.data(), &model::ApplicationStateModel::editModeChanged, this, &MatWindow::showInk);

    connect(m_model.data(), &model::ApplicationStateModel::matModeStateChanged, this,
            &MatWindow::onMatModeStateChanged, Qt::QueuedConnection);

    connect(liveCapture.data(), &model::LiveCaptureModel::captureStateChanged, this, &MatWindow::onCaptureStateChanged);

    connect(m_model.data(), &model::ApplicationStateModel::monitorWindowMinimizedChanged, this, &MatWindow::onMonitorWindowMinimizedChanged);

    connect(m_model.data(), &model::ApplicationStateModel::inkWidgetChanged, this, &MatWindow::onInkWidgetChanged);

    auto settings = GlobalUtilities::applicationSettings("inkpen");

    m_inkWidget.reset(new InkWidget(nullptr, nullptr, this, GlobalUtilities::applicationSettings()->value("fps_enabled", false).toBool(), m_model));
    m_inkWidget->installEventFilter(this);
    connect(m_inkWidget.data(), &InkWidget::inkPointAdded, this, &MatWindow::onInkPointAdded);
    
    m_inkWidget->setPenMode(settings->value("pen_mode_enabled", true).toBool());

    auto inkPenSize = settings->value("inkpen_size", 10).toInt();
    m_inkWidget->penSizeChanged(inkPenSize);

    m_inkButton->setModel(m_inkWidget);
    m_inkButton->hide();
    m_inkButton->raise();
    m_inkButton->move(this->width() - 250, 100);

    m_inkButton->inkBtn()->installEventFilter(this);
    this->installEventFilter(this);
}

void MatWindow::onScreenGeometryChanged(const QRect &geometry)
{
    move(geometry.topLeft());
    resize(geometry.size());

    ui->stageViewer->resize(geometry.size());

    if (m_inkWidget)
    {
        m_inkWidget->setGeometry(ui->stageViewer->geometry());
    }
}

MatWindow::~MatWindow()
{
    m_inkButton.reset();

    delete ui;
}

bool MatWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (m_inkButton)
    {
        if (this == obj)
        {
            switch (event->type())
            {
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
                QMouseEvent* e = static_cast<QMouseEvent*>(event);
                QSharedPointer<StyledColorDialog> scd = m_inkButton->colorPicker();
                if (!scd->rect().contains(scd->mapFromParent(e->pos())) &&
                        !m_inkButton->rect().contains(m_inkButton->mapFromParent(e->pos())))
                {
                    m_inkButton->colorBtn()->setChecked(false);
                }
            }
            return QMainWindow::eventFilter(obj, event);
        }
        else if (m_inkButton->inkBtn() == obj)
        {
            static QPoint lastPnt;
            static bool isHover = false;
            if(event->type() == QEvent::MouseButtonPress)
            {
                m_inkButton->setInkClick(true);
                QMouseEvent* e = static_cast<QMouseEvent*>(event);
                if(m_inkButton->rect().contains(e->pos()) && e->button() == Qt::LeftButton)
                {
                    lastPnt = e->pos();
                    isHover = true;
                }
            }
            else if(event->type() == QEvent::MouseMove && isHover)
            {
                m_inkButton->setInkClick(false);
                QMouseEvent* e = static_cast<QMouseEvent*>(event);
                int x = m_inkButton->x() + e->pos().x() - lastPnt.x();
                int y = m_inkButton->y() + e->pos().y() - lastPnt.y();
                if (this->rect().contains(x, y) && x < width() - m_inkButton->width() && y < height() - m_inkButton->height())
                {
                    m_inkButton->move(x, y);
                }
            }
            else if(event->type() == QEvent::MouseButtonRelease && isHover)
            {
                isHover = false;
            }
        }
    }

    return false;
}

void MatWindow::onModeChanged()
{
    onMatModeStateChanged(m_model->matModeState());
}

void MatWindow::onCaptureStateChanged()
{
    if(m_inkWidget)
    {
        QSharedPointer<InkData> inkData;

        switch (m_model->mode())
        {
        case model::ApplicationStateModel::Mode::LiveCapture:
            if (m_model->liveCapture()->selectedVideoStreamSources().contains(common::VideoSourceInfo::DownwardFacingCamera())) {
                inkData = m_model->liveCapture()->inkData();
            }
            break;
        case model::ApplicationStateModel::Mode::Preview:
            if (auto selectedProject = m_model->selectedProject())
            {
                inkData = selectedProject->inkData();
            }
            break;
        default:
            break;
        }

        showInk();

        if(inkData && inkData != m_inkData)
        {
            m_inkWidget->setInkData(inkData);
            m_inkData = inkData;
        }
    }

    onViewportChanged();
}

void MatWindow::onViewportChanged()
{
    QRectF viewport(0, 0, 1, 1);
    auto liveCapture = m_model->liveCapture();

    switch (m_model->mode())
    {
    case model::ApplicationStateModel::Mode::LiveCapture:
    {
        // Viewfinder is shown only for downward facing camera while not capturing
        if (liveCapture->captureState() == model::LiveCaptureModel::CaptureState::NotCapturing) {
            static QVector<common::VideoSourceInfo> sourcesWithVisibleViewport( { common::VideoSourceInfo::DownwardFacingCamera(),
                                                                                  common::VideoSourceInfo::SproutCamera() } );

            for(auto videoStream : liveCapture->videoStreamSources()) {
                const auto sourceType = videoStream->videoSource();
                if (sourcesWithVisibleViewport.contains(sourceType) && liveCapture->selectedVideoStreamSources().contains(sourceType)) {
                    viewport = liveCapture->viewport();
                    break;
                }
            }
        }

        QSize frameSize = m_compositor->frameSize();
        if (!frameSize.isValid())
            frameSize = size();
        calcInkWidgetSize(frameSize.scaled(size(), Qt::KeepAspectRatio));
        break;
    }
    case model::ApplicationStateModel::Mode::Preview:
        // SPROUT-18237 - Viewport should be visible only in reprojection mode
        if (m_model->matModeState() == model::ApplicationStateModel::MatModeState::Reprojection)
        {
            viewport = m_model->postCapture()->viewport();
        }

        auto selectedProject = m_model->selectedProject();
        if (selectedProject && !selectedProject->items().empty())
        {
            auto item = selectedProject->items().first();
            QRect targetRect;
            targetRect.setSize(item->imageRect().size().scaled(this->size(), Qt::KeepAspectRatio));
            targetRect.moveCenter(this->rect().center());
            ui->stageViewer->setGeometry(targetRect);

            if (m_inkWidget)
            {
                calcInkWidgetSize(targetRect.size());
            }
        }
        break;
    }

    bool isViewfinderVisible = viewport.width() < 1 || viewport.height() < 1;

    if (isViewfinderVisible)
    {
        QPoint shift(0.0, 0.0);
        auto currentSize = this->size();

        auto selectedProject = m_model->selectedProject();
        if (selectedProject) {
            if(m_model->mode() == model::ApplicationStateModel::Mode::Preview)
            {
                QSize firstItemImageSize = selectedProject->items().first()->metadata()->geometry().size();
                currentSize = firstItemImageSize.scaled(currentSize, Qt::KeepAspectRatio);
            }

            currentSize.scale(this->size(), Qt::KeepAspectRatio);
            shift.setX(static_cast<int>(this->width() - currentSize.width()) / 2.0);
            shift.setY(static_cast<int>(this->height() - currentSize.height()) / 2.0);
        }

        auto matViewport = common::Utilities::calculateAbsoluteViewport(viewport, currentSize);

        auto topLeft = matViewport.topLeft();
        auto bottomRight = matViewport.bottomRight();

        // Now we have real viewport (what camera is looking at) however we need to make sure that viewport is not in view

        topLeft.setX(topLeft.x() - ui->viewportIndicator->lineWidth() + shift.x());
        topLeft.setY(topLeft.y() - ui->viewportIndicator->lineWidth() + shift.y());
        bottomRight.setX(bottomRight.x() + ui->viewportIndicator->lineWidth() + shift.x());
        bottomRight.setY(bottomRight.y() + ui->viewportIndicator->lineWidth() + shift.y());

        if (matViewport != rect()) {
            topLeft.setX(std::max(topLeft.x(), 0));
            topLeft.setY(std::max(topLeft.y(), 0));
            bottomRight.setX(std::min(bottomRight.x(), this->width()));
            bottomRight.setY(std::min(bottomRight.y(), this->height()));
        }

        // Inverse the Y for the live stream
        int height = bottomRight.y() - topLeft.y();
        ui->viewportIndicator->move(QPoint(topLeft.x(), topLeft.y()));
        ui->viewportIndicator->resize(QSize(bottomRight.x() - topLeft.x(), height));
    }

    ui->viewportIndicator->setStyleSheet(ViewfinderStylesheets[isViewfinderVisible]);
    ui->viewportIndicator->repaint();
    ui->viewportIndicator->update();
}

void MatWindow::onMonitorWindowMinimizedChanged(bool monitorWindowMinimized)
{
    if (!m_model->presentationMode())
    {
        if (monitorWindowMinimized)
        {
            hide();
        }
        else
        {
            show();
        }
    }
}

void MatWindow::onMatModeStateChanged(model::ApplicationStateModel::MatModeState state)
{
    QString styleSheet = BlackBackgroundStylesheet;

    switch (state) {
    case model::ApplicationStateModel::MatModeState::TransitioningToDesktop:
    case model::ApplicationStateModel::MatModeState::Desktop:
    case model::ApplicationStateModel::MatModeState::TransitioningToNone:
    case model::ApplicationStateModel::MatModeState::None:
        styleSheet = TransparentBackgroundStylesheet;
        break;
    case model::ApplicationStateModel::MatModeState::TransitioningToLampOn:
    case model::ApplicationStateModel::MatModeState::LampOn:
    case model::ApplicationStateModel::MatModeState::Flash:
        styleSheet = WhiteBackgroundStylesheet;
        break;
    case model::ApplicationStateModel::MatModeState::TransitioningToFlash:
        // Keep current value
        styleSheet = ui->centralWidget->styleSheet();
        break;
    }

    if (state == model::ApplicationStateModel::MatModeState::TransitioningToReprojection)
    {
        onSelectedProjectChanged(m_model->selectedProject());
    }

    bool bReprojection = (state == model::ApplicationStateModel::MatModeState::Reprojection) &&
            (m_model->mode() == model::ApplicationStateModel::Mode::Preview);

    showInk();

    ui->centralWidget->setStyleSheet(styleSheet);
    if(m_model->singleScreenMode())
    {
        setVisible(false);
    }
    else
    {
        if(GlobalUtilities::findScreenGeometry(GlobalUtilities::MatScreen).isNull())
        {
            setVisible(false);
        }
        else
        {
            setVisible(styleSheet != TransparentBackgroundStylesheet);
        }
    }
    if (m_stageProject && !m_stageProject->items().empty())
    {
        auto item = m_stageProject->items().first();
        auto editMetadata = item->metadata().dynamicCast<EditableItemMetadata>();
        if (editMetadata)
        {
            // use bReprojection when setting visibility to avoid having it visible when on LiveCapture / LampOn
            ui->stageViewer->setVisible(bReprojection);
            item->setImageMode(bReprojection ? StageItem::ImageMode::OPaque : StylesheetImageModes[styleSheet]);
            // we dont want alpha and review (background removal) on the Mat
            editMetadata->setObjectSegEnabled(false);
            editMetadata->setShowObjSegPreview(false);
        }
    }
    else
    {
        ui->stageViewer->setVisible(bReprojection);
    }

    onCaptureStateChanged();
    if ( m_model->mainWindowLocation() != model::ApplicationStateModel::MainWindowLocation::MonitorOnMat
         // For Flash we don't raise
         && state != model::ApplicationStateModel::MatModeState::TransitioningToFlash
         && state != model::ApplicationStateModel::MatModeState::Flash)
    {
        raise();
    }
}

void MatWindow::onPerspectiveQuadChanged()
{
    auto editMetadata = m_model->selectedProject()->items().first()->metadata().dynamicCast<EditableItemMetadata>();
    m_model->postCapture()->setViewport(editMetadata->viewport());
}

void MatWindow::closeEvent(QCloseEvent* event)
{
    common::Utilities::processCloseEvent(event, m_model);

    if (event->isAccepted())
    {
        emit closing();

        QMainWindow::closeEvent(event);
    }
}

bool MatWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
{
    Q_UNUSED(result)

    // Now only handle the Windows native message
    if(QString(eventType) != "windows_generic_MSG")
    {
        return false;
    }

    MSG* msg = static_cast<MSG *>(message);
    // Only handle the PEN message
    if(msg->message >= WM_NCPOINTERUPDATE && msg->message <= DM_POINTERHITTEST)
    {
        UINT32 pointerId = GET_POINTERID_WPARAM(msg->wParam);
        POINTER_INPUT_TYPE pointerType = PT_POINTER;

        if (!GetPointerType(pointerId, &pointerType))
        {
            qWarning() << "Can not get the pointer type!";
            return false;
        }

        if(pointerType == PT_TOUCH)
        {
            POINTER_TOUCH_INFO touchInfo;
            if (!GetPointerTouchInfo(pointerId, &touchInfo))
            {
                qWarning() << "Can not get touch information!";
                return false;
            }

            QPoint position(touchInfo.pointerInfo.ptPixelLocation.x, touchInfo.pointerInfo.ptPixelLocation.y);
            QPoint posInWindow = mapFromGlobal(position);

            if (m_inkWidget && m_inkWidget->isVisible())
            {
                QSharedPointer<StyledColorDialog> scd = m_inkButton->colorPicker();
                bool inkButtonIn = m_inkButton->rect().contains(m_inkButton->mapFromParent(posInWindow));
                bool styleDialogIn = scd->rect().contains(scd->mapFromParent(posInWindow));

                if (m_inkButton->isVisible())
                {
                    if(msg->message == WM_POINTERDOWN && !styleDialogIn && !inkButtonIn)
                    {
                        m_inkButton->colorBtn()->setChecked(false);
                    }
                }
            }
        }

        if(pointerType == PT_PEN)
        {
            POINTER_PEN_INFO penInfo;
            if (!GetPointerPenInfo(pointerId, &penInfo))
            {
                qWarning() << "Can not get pen information!";
                return false;
            }

            QPoint position(penInfo.pointerInfo.ptPixelLocation.x, penInfo.pointerInfo.ptPixelLocation.y);
            QPoint posInWindow = mapFromGlobal(position);

            if (m_inkWidget && m_inkWidget->isVisible())
            {
                QSharedPointer<StyledColorDialog> scd = m_inkButton->colorPicker();
                bool inkButtonIn = m_inkButton->rect().contains(m_inkButton->mapFromParent(posInWindow));
                bool styleDialogIn = scd->rect().contains(scd->mapFromParent(posInWindow));

                if (m_inkButton->isVisible())
                {
                    if((msg->message == WM_POINTERENTER || msg->message == WM_POINTERDOWN) && inkButtonIn)
                    {
                        m_inkStart = true;
                    }

                    if(m_inkStart)
                    {
                        if(scd->isVisible() && (inkButtonIn || styleDialogIn))
                        {
                            return false;
                        }
                        else if(!scd->isVisible() && inkButtonIn)
                        {
                            return false;
                        }
                    }

                    if(msg->message == WM_POINTERDOWN)
                    {
                        m_inkButton->colorBtn()->setChecked(false);
                    }

                    if(msg->message != WM_POINTERENTER && msg->message != WM_POINTERUPDATE)
                    {
                        m_inkStart = false;
                    }
                }

                switch (msg->message)
                {
                case WM_POINTERDOWN:
                    emit penPressDown(penInfo);
                    m_model->liveCapture()->setInking(true);
                    break;
                case WM_POINTERUP:
                    emit penPressUp(penInfo);
                    m_model->liveCapture()->setInking(false);
                    break;
                case WM_POINTERENTER:
                    emit penHoverEntered(penInfo);
                    break;
                case WM_POINTERLEAVE:
                    emit penHoverExited(penInfo);
                    break;
                case WM_POINTERUPDATE:
                    emit penMove(penInfo);
                    break;
                };
            }

            // If it is necessary, we 'ate' the message and do not pass it to the QT system.
            if (posInWindow.x() > 0 && posInWindow.y() > 0)
            {
                *((LRESULT*)result) = 0;
                return true;
            }
        }
    }

    return false;
}

void MatWindow::calcInkWidgetSize(const QSize& scaledImageSize)
{
    if (!m_inkWidget)
        return;

    if (m_inkWidget->size() != scaledImageSize)
    {
        int newX = 0;
        int newY = 0;
        //we need to calculate correct size for ink widget
        if (size().width() > scaledImageSize.width())
        {
            newX = (size().width() - scaledImageSize.width()) / 2;
        }
        else
        {
            newY = (size().height() - scaledImageSize.height()) / 2;
        }

        m_inkWidget->resize(scaledImageSize.width(), scaledImageSize.height());
        m_inkWidget->move(newX, newY);
        m_inkWidget->updatePixmapBySize();
    }

    if(m_inkData)
        m_inkData->setCanvasSize(m_inkWidget->size());
}

void MatWindow::onInkWidgetChanged()
{
    if(m_inkWidget)
        m_inkWidget->updatePixmap(true);
}

void MatWindow::onDisplayCountChanged(int screenCount)
{
    BaseWindow::onDisplayCountChanged(screenCount);

    if (!GlobalUtilities::findScreenGeometry(GlobalUtilities::MatScreen).isNull() &&
            !m_model->singleScreenMode())
    {
        auto matScreen = GlobalUtilities::findScreen(GlobalUtilities::MatScreen);
        if(matScreen !=nullptr)
        {
            connect(matScreen, &QScreen::geometryChanged, this, &MatWindow::onScreenGeometryChanged);
            onScreenGeometryChanged(matScreen->geometry());
        }

        onMatModeStateChanged(m_model->matModeState());

        m_inkButton->move(this->width() - 250, 100);
    }
    else
    {
        hide();
    }
}

QScreen* MatWindow::findOwnScreen()
{
    return GlobalUtilities::findScreen(GlobalUtilities::MatScreen);
}

StageViewer::Options MatWindow::stageViewerOptions()
{
    return StageViewer::DisableControls | BaseWindow::stageViewerOptions();
}

void MatWindow::applySelectedProject(QSharedPointer<StageProject> selectedProject)
{
    for (auto connection : m_itemConnections)
    {
        disconnect(connection);
    }

    if (selectedProject)
    {
        auto item = selectedProject->items().first();
        if (m_inkWidget)
        {
            QSharedPointer<InkData> inkData = selectedProject->inkData();
            if (inkData && inkData != m_inkData)
            {
                m_inkWidget->setInkData(inkData);
                m_inkData = inkData;
                QRect targetRect;
                targetRect.setSize(item->imageRect().size().scaled(this->size(), Qt::KeepAspectRatio));
                calcInkWidgetSize(targetRect.size());
            }
        }

        auto editMetadata = item->metadata().dynamicCast<EditableItemMetadata>();
        if (editMetadata)
        {
            m_stageProject->clone(*selectedProject);
            m_stageProject->setSize(size());

            auto item_original = m_stageProject->items().first();
            if (item_original)
            {
                auto editMetadata_original = item_original->metadata().dynamicCast<EditableItemMetadata>();
                if (editMetadata_original) {
                    editMetadata_original->setGeometry(rect());
                    editMetadata_original->setViewport(QRectF(0, 0, 1, 1));
                    // we dont want alpha and review (background removal) on the Mat
                    editMetadata_original->setObjectSegEnabled(false);
                    editMetadata_original->setShowObjSegPreview(false);
                }

                m_itemConnections << connect(item_original->metadata().data(), &StageItemMetadata::pespectiveChanged, m_inkWidget.data(), &InkWidget::onUpdate);
            }

            ui->stageViewer->setStage(m_stageProject, stageViewerOptions());

            m_itemConnections << connect(editMetadata.data(), &EditableItemMetadata::perspectiveQuadChanged, this, &MatWindow::onPerspectiveQuadChanged);
            m_itemConnections << connect(editMetadata.data(), &EditableItemMetadata::changed, this, &MatWindow::onMetadataChanged);
            m_itemConnections << connect(editMetadata.data(), &EditableItemMetadata::stageItemCropDataChanged, this, &MatWindow::onMetadataChanged);
            m_itemConnections << connect(item->metadata().data(), &StageItemMetadata::pespectiveChanged, m_inkWidget.data(), &InkWidget::onUpdate);

            m_inkWidget->onUpdate();
        }
    }
}

void MatWindow::onMetadataChanged()
{
    if (m_stageProject && m_model && m_model->selectedProject() && !m_model->selectedProject()->items().empty())
    {
        auto editMetadata = m_model->selectedProject()->items().first()->metadata().dynamicCast<EditableItemMetadata>();
        auto editMetadata_mat = m_stageProject->items().first()->metadata().dynamicCast<EditableItemMetadata>();
        if (editMetadata && editMetadata_mat)
        {
            editMetadata_mat->update(editMetadata.data(), false, false);
            // we dont want alpha and review (background removal) on the Mat
            editMetadata_mat->setObjectSegEnabled(false);
            editMetadata_mat->setShowObjSegPreview(false);

            /// NOTE TO DEVELOPERS: We replace the perspective always as in both Doc and Normal models, we should update the item UVs.
            if(editMetadata->mode() == StageItemMetadata::ModeDocument)
            {
                editMetadata_mat->setPerspectiveQuadOrder(editMetadata->perspectiveQuadOrder());
                editMetadata_mat->setPerspectiveQuad(editMetadata->perspectiveQuad());
            }
        }
    }
}

void MatWindow::onSelectedVideoStreamSourcesChanged()
{
    showInk();
}

void MatWindow::showInk()
{
    if(m_inkWidget && m_inkButton)
    {
        const auto matState = m_model->matModeState();
        bool isVisibleOnLiveView = m_model->mode() == model::ApplicationStateModel::Mode::LiveCapture &&
                m_model->liveCapture()->captureState() == model::LiveCaptureModel::NotCapturing &&
                m_model->liveCapture()->selectedVideoStreamSources().contains(common::VideoSourceInfo::DownwardFacingCamera()) &&
                (matState == model::ApplicationStateModel::MatModeState::LampOff ||
                 matState == model::ApplicationStateModel::MatModeState::LampOn ||
                 matState == model::ApplicationStateModel::MatModeState::TransitioningToLampOff ||
                 matState == model::ApplicationStateModel::MatModeState::TransitioningToLampOn);

        bool isVisibleOnPreview = m_model->mode() == model::ApplicationStateModel::Mode::Preview &&
                matState == model::ApplicationStateModel::Reprojection;

        bool isEditMode = false;//m_model->isEditMode();

        bool visible = !isEditMode && (isVisibleOnLiveView || isVisibleOnPreview);
        m_inkWidget->setVisible(visible);
        m_inkButton->setVisible(visible);
    }
}

void MatWindow::onInkPointAdded(const QPoint& point, double width)
{
    Q_UNUSED(point);
    Q_UNUSED(width);

    if (m_inkWidget && m_inkWidget->isVisible())
    {
        m_inkWidget->repaint();
    }
}

} // namespace mat
} // namespace capture
