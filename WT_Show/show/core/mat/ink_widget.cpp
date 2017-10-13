#include "ink_widget.h"

#include <QtWidgets>
#include <QPainter>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include "frame_counting.h"
#include "common/ink_manger.h"
#include "capture_item_metadata.h"
#include "common/utilities.h"
#include "common/history_manager.h"
#include "object_ink_transform.h"

namespace capture {
namespace mat {

#define SMALL_PEN_SIZE 10
#define ERASER_SIZE 30
#define BASE_PRESSURE   (1024/2)

InkWidget::InkWidget(QWidget* leftWidget, QWidget* rightWidget, QWidget *parent, bool showFps, QSharedPointer<model::ApplicationStateModel> model)
  : QWidget(parent)
  , m_color(Qt::yellow)
  , m_basePenWidth(SMALL_PEN_SIZE)
  , m_eraserSize(ERASER_SIZE)
  , m_rightMenu(rightWidget)
  , m_leftMenu(leftWidget)
  , m_eraserMode(false)
  , m_penEraserMode(false)
  , m_penMode(true)
  , m_penDrawing(false)
  , m_enablePen(true)
  , m_enableRemoveStroke(true)
  , m_mouseDrawing(false)
  , m_penPointColor(Qt::black)
  , m_enabled(true)
  , m_model(model)
{
    // lets make sure we have a transparent background
    setAutoFillBackground(false);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);

    // and get prepared to adapt if main window get resized
    connect(parent->window(), SIGNAL(windowsResized(QSize)), this, SLOT(mainWindowResize(QSize)));
    connect(parent->window(), SIGNAL(penHoverEntered(const POINTER_PEN_INFO&)),
            this, SLOT(onPenHoverEntered(const POINTER_PEN_INFO&)));
    connect(parent->window(), SIGNAL(penHoverExited(const POINTER_PEN_INFO&)),
            this, SLOT(onPenHoverExited(const POINTER_PEN_INFO&)));
    connect(parent->window(), SIGNAL(penPressDown(const POINTER_PEN_INFO&)),
            this, SLOT(onPenDown(const POINTER_PEN_INFO&)));
    connect(parent->window(), SIGNAL(penPressUp(const POINTER_PEN_INFO&)),
            this, SLOT(onPenUp(const POINTER_PEN_INFO&)));
    connect(parent->window(), SIGNAL(penMove(const POINTER_PEN_INFO&)),
            this, SLOT(onPenMove(const POINTER_PEN_INFO&)));

    connect(m_model.data(), &model::ApplicationStateModel::editModeChanged, this, &InkWidget::onEditModeChanged);

    // set timer
    m_resizeTimer.setSingleShot(true);
    connect(&m_resizeTimer, SIGNAL(timeout()), this, SLOT(resizeToWindow()));

    // Set Object Name
    setObjectName("InkWidget");

    // Set its geometry
    resizeToWindow();

    // Enter pen mode
    setPenMode(true);

    m_frameCounting = QSharedPointer<FrameCounting>::create(showFps, this);
}

void InkWidget::setPenMode(bool penMode)
{
    m_penMode = penMode;
    setAttribute(Qt::WA_TransparentForMouseEvents, !m_enabled || m_penMode);
}

void InkWidget::eraseStroke(const QPoint& pos)
{
    if(!m_enableRemoveStroke)
    {
        emit inkDataErasing(mapToGlobal(pos));
        return;
    }

    if(m_inkData.isNull())
    {
        qWarning() << "Please initialize the ink data firstly!";
        return;
    }

    auto ptParent = mapToParent(pos);
    if (!geometry().contains(ptParent))
        return;

    if(!m_forward)
    {
        bool erased = false;
        if (m_model->mode() == model::ApplicationStateModel::Mode::Preview)
        {
            auto stageProject = m_model->selectedProject();
            auto stageItem = stageProject->items().first();

            QTransform tran;
            if (!ObjectInkTransform::imageNormalToWnd(geometry().size(), corner(), tran))
            {
                return;
            }

            EditableItemMetadata* metadata = (EditableItemMetadata*)stageItem->metadata().data();
            auto inkStrokes_old = metadata->inkStrokes();
            auto inkStrokes = common::InkManger::applyPerspective(inkStrokes_old, stageItem);
            for (int i = 0; i < inkStrokes.count(); ++i)
            {
                QJsonObject strokeObj = inkStrokes[i].toObject();
                ObjectInkStroke inkStroke(strokeObj);
                QPolygonF points = inkStroke.points();

                for (int j = 0; j < points.count(); ++j)
                {
                    if (QPointF(pos - tran.map(points[j])).manhattanLength() < m_eraserSize)
                    {
                        inkStrokes.removeAt(i);
                        inkStrokes_old.removeAt(i);
                        metadata->setInkStrokes(inkStrokes_old);
                        updatePixmap(true);
                        erased = true;
                        break;
                    }
                }

                if (erased)
                    break;
            }
        }
        else
        {
            int strokeCount = m_inkData->strokeCount();
            for (int i = strokeCount - 1; i >= 0; i--)
            {
                auto stroke = m_inkData->stroke(i);
                int ptCount = stroke->pointCount();
                for (int j = 0; j < ptCount; j++)
                {
                    auto inkPt = stroke->point(j);
                    if (QPoint(pos - inkPt.point).manhattanLength() < m_eraserSize)
                    {
                        m_inkData->removeStroke(i);
                        updatePixmap(true);
                        erased = true;
                        break;
                    }
                }

                if (erased)
                    break;
            }
        }

        if (erased) 
            update();

        emit inkDataErasing(pos);
    }
}

void InkWidget::setInkData(QSharedPointer<InkData> strokes)
{
    m_inkData = strokes;
    m_pixmap = QPixmap(width(), height());
    m_pixmap.fill(QColor(0, 0, 0, 0));

    emit inkDataChanged(strokes);

    updatePixmap(true);

    connect(m_model.data(), &model::ApplicationStateModel::editModeChanged, this, &InkWidget::onUpdate);
}

void InkWidget::setStageRenderer(StageRenderer* renderer)
{
    m_renderer = renderer;
}

void InkWidget::onUpdate()
{
    updatePixmap(true);
}

void InkWidget::setColor(const QColor &c)
{
    if (c.isValid())
        m_color = c;
}

void InkWidget::setPenPointColor(const QColor &penPointColor)
{
    if (penPointColor.isValid())
    {
        m_penPointColor = penPointColor;
        changeCursor(m_basePenWidth);
    }
}

QColor InkWidget::penPointColor() const
{
    return m_penPointColor;
}

QColor InkWidget::color() const
{
    return m_color;
}

void InkWidget::updatePixmap(bool updateAll)
{
    if(m_inkData.isNull())
    {
        return;
    }

    QPainter painter(&m_pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    auto region = QRect(0, 0, width(), height());

    if(updateAll)
        painter.fillRect(region, Qt::transparent);
    if(m_model->mode() == model::ApplicationStateModel::Mode::Preview)
    {
        QTransform tran;
        auto corners = corner();
        if(!ObjectInkTransform::imageNormalToWnd(geometry().size(), corner(), tran))
        {
            painter.restore();
            return;
        }

        painter.setTransform(tran);

        auto stageProject = m_model->selectedProject();
        auto stageItem = stageProject->items().first();
        EditableItemMetadata* metadata = (EditableItemMetadata*)stageItem->metadata().data();

        QPolygonF perspectiveQuad = metadata->correctedPerspectiveQuad();
        double penScale = 1.0 / ScalableRect::perspectiveSize(perspectiveQuad).width();

        if(updateAll)
        {
            auto allInkStrokes = metadata->inkStrokes();
            allInkStrokes = common::InkManger::applyPerspective(allInkStrokes, stageItem);
            for(auto strokeJson : allInkStrokes)
            {
                QJsonObject strokeObj = strokeJson.toObject();
                ObjectInkStroke inkStroke(strokeObj);
                inkStroke.draw(painter, penScale);
            }
        }
        else
        {
            QJsonArray lastInkStroke;
            lastInkStroke.push_back(metadata->inkStrokes().last());
            lastInkStroke = common::InkManger::applyPerspective(lastInkStroke, stageItem);
            auto strokeJson = lastInkStroke.last();
            QJsonObject strokeObj = strokeJson.toObject();
            ObjectInkStroke inkStroke(strokeObj);
            inkStroke.draw(painter, penScale);
        }
    }
    else
    {
        if (updateAll)
        {
            for (int i = 0; i < m_inkData->strokeCount(); i++)
            {
                auto stroke = m_inkData->stroke(i);
                stroke->draw(painter);
            }
        }
        else
        {
            auto stroke = m_inkData->stroke(m_inkData->strokeCount()-1);
            stroke->draw(painter);
        }
    }

    m_model->setInkPixMap(m_pixmap);

    QMetaObject::invokeMethod(this, "repaint", Qt::QueuedConnection);
}

void InkWidget::paintEvent(QPaintEvent *paintEvent)
{
    auto counter = m_frameCounting->count();

    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRect clipRect = paintEvent->rect();

    // Draw saved strokes
    if(m_inkData.isNull())
    {
        qWarning() << "Please initialize the ink data firstly!";
        return;
    }
    else
    {
        painter.drawPixmap(clipRect, m_pixmap, clipRect);
    }

    // Draw current stroke
    auto currentStroke = m_inkData->currentStroke();

    if(currentStroke->pointCount() > 0 && currentStroke->boundRect().intersects(clipRect)) 
	{
        if(m_model->mode() == model::ApplicationStateModel::Mode::Preview)
        {
            QTransform tran;
            if(!ObjectInkTransform::imageNormalToWnd(geometry().size(), corner(), tran))
            {
                painter.restore();
                return;
            }
            else
            {
                auto stageProject = m_model->selectedProject();
                auto stageItem = stageProject->items().first();
                EditableItemMetadata* metadata = (EditableItemMetadata*)stageItem->metadata().data();
                QPolygonF perspectiveQuad = metadata->correctedPerspectiveQuad();

                ObjectInkStroke objStroke = common::InkManger::convertInkStroke(currentStroke, stageItem, corner());
                painter.setTransform(tran);
                double penScale = 1.0 / common::InkManger::perspectiveSize(perspectiveQuad).width();

                auto imageSize = stageItem->originalImage().size();

                QTransform transform;
                if(!ObjectInkTransform::applyPerspectiveOnImage(geometry().size(), imageSize, perspectiveQuad, transform))
                {
                    return;
                }
                ObjectInkStroke inkStroke = ObjectInkStroke(objStroke.color(), transform.map(objStroke.points()), objStroke.penSize());
                double newInkPenWidth = inkStroke.penSize().first() * penScale;
                double inkDataPenWidth = currentStroke->point(0).size;

                QPointF newInkPoint = tran.map(QPointF(newInkPenWidth, newInkPenWidth));
                double scale = newInkPoint.x() / inkDataPenWidth;
                m_model->setInkPenWidthScale(scale);
                inkStroke.draw(painter, penScale);
            }
            painter.restore();
        }
        else
        {
            currentStroke->draw(painter);
        }
    }
}

void InkWidget::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "mouseMoveEvent: " << event;
    if(m_eraserMode)
    {
        // Erase Line
        eraseStroke(event->localPos().toPoint());
    }
    else if(!m_penDrawing)
    {
        if(!m_mouseDrawing) return;
        addPoint(event->localPos().toPoint(), m_basePenWidth);
    }

    event->accept();
}

void InkWidget::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "mousePressEvent" << event;
    if(m_eraserMode)
    {
        // Erase Line
        eraseStroke(event->localPos().toPoint());
    }
    else if(!m_penDrawing)
    {
        m_mouseDrawing = true;
        addPoint(event->localPos().toPoint(), m_basePenWidth);
    }

    event->accept();
}

void InkWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "mouseReleaseEvent" << event;
    if(m_eraserMode)
    {
        // Erase Line
        eraseStroke(event->localPos().toPoint());
    }
    else if(!m_penDrawing)
    {
        if(!m_mouseDrawing) return;

        addPoint(event->localPos().toPoint(), m_basePenWidth);
        addStroke();
        m_mouseDrawing = false;
    }

    event->accept();
}

void InkWidget::onPenHoverEntered(const POINTER_PEN_INFO& /*penInfo*/)
{
    //qDebug() << "Pen enter Hover";
}

void InkWidget::onPenHoverExited(const POINTER_PEN_INFO& /*penInfo*/)
{
    //qDebug() << "Pen exit Hover";
}

QString InkWidget::penInfoStr(const POINTER_PEN_INFO& penInfo)
{
    QPoint position(penInfo.pointerInfo.ptPixelLocation.x, penInfo.pointerInfo.ptPixelLocation.y);
    QString result = QString("pressure: %1; Position [%2, %3]").arg(penInfo.pressure)
            .arg(mapFromGlobal(position).x())
            .arg(mapFromGlobal(position).y());

    if((penInfo.penFlags & PEN_FLAG_ERASER) || (penInfo.penFlags & PEN_FLAG_INVERTED))
    {
        result += "; Eraser button has pressed";
    }
    else if(penInfo.penFlags & PEN_FLAG_BARREL)
    {
        result += "Barrel button has pressed";
    }

    return result;
}

void InkWidget::onPenDown(const POINTER_PEN_INFO& penInfo)
{
    //qDebug() << "Pen Down : " << penInfoStr(penInfo);
    if (m_enabled && m_enablePen && penInfo.pressure > 0)
    {
        QPoint posInGlobal(penInfo.pointerInfo.ptPixelLocation.x, penInfo.pointerInfo.ptPixelLocation.y);

        QPoint pt(mapFromGlobal(posInGlobal));
        auto ptParent = mapToParent(pt);
        if (!geometry().contains(ptParent))
            return;

        bool bOutside=true;
        if (m_leftMenu && m_rightMenu)
        {
            QPoint ptInRightMenuParent = m_rightMenu->parentWidget()->mapFromGlobal(posInGlobal);
            QPoint ptInLeftMenuParent = m_leftMenu->parentWidget()->mapFromGlobal(posInGlobal);
            if(m_rightMenu->geometry().contains(ptInRightMenuParent)
                    || m_leftMenu->geometry().contains(ptInLeftMenuParent))
            {
                bOutside = false;
            }
        }

        if(!bOutside)
        {
            return;
        }

        m_penDrawing = true;

        // Drawing with mouse at this moment.
        if(m_inkData->currentStroke()->pointCount() > 0)
        {
            addStroke();
        }

        if(m_eraserMode)
        {
            // Erase line
            eraseStroke(pt);
        }
        else if((penInfo.penFlags & PEN_FLAG_ERASER) || (penInfo.penFlags & PEN_FLAG_INVERTED))
        {
            // Entering Pen Eraser Mode
            m_penEraserMode = true;

            // Erase line
            eraseStroke(pt);
        }
        else if(penInfo.penFlags & PEN_FLAG_BARREL)
        {
            // Pen barrel button has been pressed.
        }
        else
        {
            double width = m_basePenWidth * penInfo.pressure * 1.0 / BASE_PRESSURE;
            addPoint(pt, width);
        }
    }
}

void InkWidget::onPenUp(const POINTER_PEN_INFO& penInfo)
{
    Q_UNUSED(penInfo)

    if(!m_penDrawing) return;

    //qDebug() << "Pen Up" << penInfoStr(penInfo);
    if(m_penEraserMode)
    {
        m_penEraserMode = false;
    }
    else
    {
        addStroke();
    }

    m_penDrawing = false;
}

void InkWidget::onPenMove(const POINTER_PEN_INFO& penInfo)
{
    //qDebug() << "Pen move: "  << penInfoStr(penInfo);
    if (m_enabled && m_enablePen && penInfo.pressure > 0)
    {
        QPoint posInGlobal(penInfo.pointerInfo.ptPixelLocation.x, penInfo.pointerInfo.ptPixelLocation.y);
        QPoint pt(mapFromGlobal(posInGlobal));

        auto ptParent = mapToParent(pt);
        if (!geometry().contains(ptParent))
            return;

        if(m_penEraserMode || m_eraserMode)
        {
            // Erase line
            eraseStroke(pt);
        }
        else
        {
            double width = m_basePenWidth * penInfo.pressure * 1.0 / BASE_PRESSURE;
            addPoint(pt, width);
        }
    }
}

void InkWidget::addPoint(const QPoint& point, double width)
{
    if(width > 0)
    {
        if(!m_forward)
        {
            auto currentStroke = m_inkData->currentStroke();
            currentStroke->addPoint(point, width);
            currentStroke->setColor(m_color);

            update(currentStroke->boundRect());
        }

        emit inkPointAdded(point, width);
    }
}

void InkWidget::addStroke()
{
    if(m_inkData.isNull())
    {
        qWarning() << "Please initialize the ink data first!";
        return;
    }

    if(m_inkData->currentStroke()->pointCount() > 0)
    {
        if(!m_forward)
        {
            auto r = m_inkData->currentStroke()->boundRect();
            if(m_model->mode() == model::ApplicationStateModel::Mode::Preview)
            {
                auto stageProject = m_model->selectedProject();
                auto stageItem = stageProject->items().first();

                common::InkManger::addStroke(m_inkData->currentStroke(), stageItem, corner());
            }
            m_inkData->addCurrentStroke1();
            updatePixmap(false);
        }

        emit inkStrokeAdded();
    }
}

void InkWidget::updateColor(const QColor& color)
{
    m_color = QColor(color);
}

void InkWidget::mainWindowResize(QSize /*size*/)
{
    //as resize events can keep coming and we dont want to update all the time
    // lets make a single shot timer and handle after some miliseconds
    m_resizeTimer.start(20);
}

void InkWidget::resizeToWindow()
{
    // we always reset the position... I guess just for the sake of doing it
    int leftMenu_width = 0;
    if(m_leftMenu)
    {
        leftMenu_width = m_leftMenu->minimumWidth();
        this->move(leftMenu_width, 0);
    }

    int rightMenu_width = 0;
    if(m_rightMenu)
    {
        rightMenu_width = m_rightMenu->minimumWidth();
    }

    resize(this->window()->size().width() - leftMenu_width - rightMenu_width,
           this->window()->size().height());
    m_pixmap = QPixmap(width(), height());
    updatePixmap(true);

    // and make sure we update the widget
    this->update();
}

void InkWidget::penSizeChanged(int penSize)
{
	m_basePenWidth = penSize;
	changeCursor(m_basePenWidth);
}

void InkWidget::colorChanged(const QColor& color)
{
	updateColor(color);
}

void InkWidget::enterEraserMode()
{
    if(m_eraserMode) return;

    changeCursor(m_eraserSize);
    update();

    m_eraserMode = true;
}

void InkWidget::enablePen(bool enable)
{
    m_enablePen = enable;
}

void InkWidget::enableRemoveStroke(bool enable)
{
    m_enableRemoveStroke = enable;
}

void InkWidget::enterDrawMode()
{
	changeCursor(m_basePenWidth);

    if(m_eraserMode)
    {
        m_eraserMode = false;
    }
}

void InkWidget::changeCursor(int penSize)
{
    QPixmap *pix = new QPixmap(penSize + 2, penSize + 2);
    pix->fill(Qt::transparent);
    QPainter *paint = new QPainter(pix);
    paint->setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
    QPen pen(m_penPointColor);
    pen.setWidth(2);
    paint->setPen(pen);
    paint->drawEllipse(2, 2, penSize - 2, penSize - 2);
    QCursor cursor = QCursor(*pix, -penSize/2, -penSize/2);
    setCursor(cursor);
}

void InkWidget::hideEvent(QHideEvent *event)
{
    m_enablePen = false;

    return QWidget::hideEvent(event);
}

void InkWidget::showEvent(QShowEvent *event)
{
    m_enablePen = true;

    return QWidget::showEvent(event);
}

void InkWidget::onEditModeChanged(model::ApplicationStateModel::EditMenuMode mode)
{
    m_enabled = mode != model::ApplicationStateModel::EditMenuMode::SubMenuOpen;
    setAttribute(Qt::WA_TransparentForMouseEvents, !m_enabled || m_penMode);
}

void InkWidget::updatePixmapBySize()
{
    m_pixmap = QPixmap(width(), height());
    m_pixmap.fill(QColor(0, 0, 0, 0));
    updatePixmap(true);
}

QPolygonF InkWidget::corner()
{
    QRectF rect(0, 0, geometry().width(), geometry().height());
    QPolygonF corner(rect);
    corner.removeLast();
    return corner;
}

} // namespace mat
} // namespace capture

