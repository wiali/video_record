#include "down_cam.h"
#include <QPainter>

DownCam::DownCam(const QSizeF &size) : m_size(size)
{
}

QRectF DownCam::boundingRect() const
{
    return QRectF(QPointF(), m_size);
}

void DownCam::setImage(const QImage& stream)
{
    m_stream = stream;
    update();
}

void DownCam::setSize(const QSizeF &size)
{
    m_size = size;
    update();
}

void DownCam::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    QRectF rect = boundingRect();

    if(!m_stream.isNull())
        painter->drawImage(rect, m_stream);
}

void DownCam::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) 
    {
        setSelected(!isSelected());
    }
}

void DownCam::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->modifiers() & Qt::ShiftModifier) 
    {
        //stuff << event->pos();
        //update();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void DownCam::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    //update();
}
