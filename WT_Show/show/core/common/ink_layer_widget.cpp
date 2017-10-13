#include "ink_layer_widget.h"

#include <QDebug>
#include <QPainter>

#include "ink_data.h"
#include "frame_counting.h"
#include <global_utilities.h>
#include "common/utilities.h"

namespace capture {
namespace common {

InkLayerWidget::InkLayerWidget(QSharedPointer<model::ApplicationStateModel> model, QWidget *parent)
    : QWidget(parent)
    , m_model(model)
    , m_renderer(nullptr)
{
    setAutoFillBackground(false);
    // lets make sure we have a transparent background
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    if (auto sv = dynamic_cast<StageViewer*>(parent))
    {
        connect(sv, &StageViewer::geometryChanged, this, &InkLayerWidget::onGeometryChanged);
    }

    connect(m_model.data(), &model::ApplicationStateModel::inkPixMapChanged, this, &InkLayerWidget::onInkPixMapChanged);

    m_frameCounting = QSharedPointer<FrameCounting>::create(
                GlobalUtilities::applicationSettings()->value("fps_enabled", false).toBool(), this);
}

void InkLayerWidget::setInkData(QSharedPointer<InkData> inkData)
{
    for(auto connection : m_inkDataConnections)
    {
        disconnect(connection);
    }

    m_inkData = inkData;

    m_inkDataConnections << connect(m_inkData.data(), &InkData::strokeAdded, this, &InkLayerWidget::onStrokeAdded);
    m_inkDataConnections << connect(m_inkData.data(), &InkData::strokeRemoved, this, &InkLayerWidget::onUpdate);

    connectCurrentStroke(m_inkData->currentStroke());
}

void InkLayerWidget::paintEvent(QPaintEvent* paintEvent)
{
    if(m_model->isEditMode())
        raise();

    auto counter = m_frameCounting->count();

    updateTransform();

    QPainter painter(this);
    painter.setTransform(m_viewportMatrix);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QRect clipRect = paintEvent->rect();

    // Recalculate clip rectangle back to inking space
    auto rect = m_inverseMatrix.mapRect(clipRect);

    // Draw saved strokes
    if(m_inkData.isNull())
    {
        qWarning() << this << "Please initialize the ink data first!";
        return;
    }
    else
    {
        painter.drawPixmap(rect, m_inkPixMap, rect);
    }

    // Draw current stroke
    auto currentStroke = m_inkData->currentStroke();

    if(currentStroke && currentStroke->pointCount() > 0 && currentStroke->boundRect().intersects(rect))
    {
        if(m_model->mode() == model::ApplicationStateModel::Mode::Preview)
        {
            currentStroke->draw(painter, m_model->inkPenWidthScale());
        }
    }

    QWidget::paintEvent(paintEvent);
}

void InkLayerWidget::setTransform(const QTransform& viewportMatrix)
{
    m_viewportMatrix = viewportMatrix;

    if (!m_renderer || !m_renderer->model())
        return;

    auto rotateCenter = m_renderer->model()->getRotateCenter();
    auto rotateAngle = m_renderer->model()->getRotateAngle().z();

    if (!qFuzzyCompare(rotateAngle, 0.0f))
    {
        QTransform transform;
        transform.translate(rotateCenter.x(), rotateCenter.y());
        transform.rotate(rotateAngle);
        transform.translate(-rotateCenter.x(), -rotateCenter.y());
        m_viewportMatrix *= transform;
    }

    m_inverseMatrix = m_viewportMatrix.inverted();
}

void InkLayerWidget::onGeometryChanged(const QRect& newRect)
{
    resize(newRect.size());
}

void InkLayerWidget::updateCurrentInkData()
{
    repaint();
}

void InkLayerWidget::updateInkDisplay()
{
    updateCurrentInkData();
}

void InkLayerWidget::connectCurrentStroke(QSharedPointer<InkStroke> stroke)
{
    if (m_inkStrokeConnection)
    {
        disconnect(m_inkStrokeConnection);
    }

    m_inkStrokeConnection = connect(stroke.data(), &InkStroke::pointAdded, this, &InkLayerWidget::onUpdate);
}

void InkLayerWidget::onStrokeAdded(QSharedPointer<InkStroke> addedStroke, QSharedPointer<InkStroke> currentStroke)
{
    connectCurrentStroke(currentStroke);
    update();
}

void InkLayerWidget::onUpdate()
{
    this->update();
}

void InkLayerWidget::setStageRenderer(StageRenderer* renderer)
{
    m_renderer = renderer;
}

void InkLayerWidget::updateTransform()
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

        QTransform matrix;
        QSize targetSize = selectedProject->inkData()->canvasSize();

        // source space is the visible portion of mat screen inside blue rectangle in mat screen coordinates
        QRectF srcRect; // topleft is (0,0)
        srcRect.setTop(targetSize.height() * vp.top());
        srcRect.setLeft(targetSize.width() * vp.left());
        srcRect.setBottom(targetSize.height() * vp.bottom());
        srcRect.setRight(targetSize.width() * vp.right());

        // make QPolygon from QRect. It wil contain 5 vertices so we remove the last one.
        QPolygonF src(srcRect);
        src.removeLast();

        // destination space is the StageViewer actual widget size
        QRectF dstRect = QRectF(rect());
        QPolygonF dst(dstRect);
        dst.removeLast();

        if (!QTransform::quadToQuad(src, dst, matrix))
        {
            throw "Failed to build transform";
        }

        setTransform(matrix);
    }
}

void InkLayerWidget::onInkPixMapChanged(QPixmap inkPixMap)
{
    m_inkPixMap = inkPixMap;
    onUpdate();
}

} // namespace common
} // namespace capture
