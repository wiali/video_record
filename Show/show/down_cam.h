#pragma once

#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

class DownCam : public QGraphicsItem
{
public:
    explicit DownCam(const QSizeF &size);

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) Q_DECL_OVERRIDE;

    void setImage(const QImage& stream);
    void setSize(const QSizeF &size);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QSizeF m_size;
    QImage m_stream;
};
