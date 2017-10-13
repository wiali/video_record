#include "ink_manger.h"

#include "model/camera_item_metadata.h"
#include "object_ink_stroke.h"
#include "ink_stroke.h"
#include "object_ink_transform.h"

namespace capture {
namespace common {

InkManger::InkManger(QObject *parent)
    : QObject(parent)
{
}

void InkManger::addStroke(QSharedPointer<InkStroke> stroke, QSharedPointer<StageItem> stageItem, const QPolygonF& corners)
{
    ObjectInkStroke objStr = convertInkStroke(stroke, stageItem, corners);

    auto metadata = stageItem->metadata();
    QJsonArray inkStrokes = metadata->inkStrokes();
    inkStrokes.push_back(objStr.toJson());
    metadata->setInkStrokes(inkStrokes);
}

ObjectInkStroke InkManger::convertInkStroke(QSharedPointer<InkStroke> inkStroke, QSharedPointer<StageItem> stageItem, const QPolygonF& corners)
{
    ObjectInkStroke objStroke;

    EditableItemMetadata* metadata = (EditableItemMetadata*)stageItem->metadata().data();

    // Get perspective quad
    QPolygonF perspectiveQuad = metadata->correctedPerspectiveQuad();
    QTransform tranWndToImageNormal;

    auto size = stageItem->metadata()->geometry().size();
    auto imageSize = stageItem->originalImage().size();
    if(!ObjectInkTransform::wndToImageNormal(size, corners, tranWndToImageNormal))
    {
        return objStroke;
    }

    QTransform transform;
    if(!ObjectInkTransform::revertPerspectiveOnImage(size, imageSize, perspectiveQuad, transform))
    {
        return objStroke;
    }
    objStroke.setColor(inkStroke->color());

    transform = tranWndToImageNormal * transform;

    double penScale = perspectiveSize(perspectiveQuad).width() / 1.0;

    for(int i = 0; i < inkStroke->pointCount(); ++i)
    {
        InkPoint inkPt = inkStroke->point(i);
        QPoint pt = inkPt.point;
        QPointF point = transform.map(QPointF(pt));
        double size = inkPt.size * penScale / 1920 ;

        objStroke.addPoint(point, size);
    }

    return objStroke;
}

QSizeF InkManger::perspectiveSize(const QPolygonF& perspectiveQuad)
{
    float top    = QVector2D(perspectiveQuad[1]).distanceToPoint(QVector2D(perspectiveQuad[0]));
    float bottom = QVector2D(perspectiveQuad[3]).distanceToPoint(QVector2D(perspectiveQuad[2]));
    float left   = QVector2D(perspectiveQuad[3]).distanceToPoint(QVector2D(perspectiveQuad[0]));
    float right  = QVector2D(perspectiveQuad[2]).distanceToPoint(QVector2D(perspectiveQuad[1]));

    return QSizeF((top+bottom)/2, (left+right)/2);
}

QJsonArray InkManger::applyPerspective(const QJsonArray &strokes, QSharedPointer<StageItem> stageItem)
{
    QJsonArray result;
    EditableItemMetadata* metadata = (EditableItemMetadata*)stageItem->metadata().data();
    QPolygonF perspectiveQuad = metadata->correctedPerspectiveQuad();
    QTransform transform;

    auto size = stageItem->metadata()->geometry().size();
    auto imageSize = stageItem->originalImage().size();
    if(!ObjectInkTransform::applyPerspectiveOnImage(size, imageSize, perspectiveQuad, transform))
    {
        return result;
    }

    for(auto strokeJson : strokes)
    {
        QJsonObject strokeObj = strokeJson.toObject();
        ObjectInkStroke inkStroke(strokeObj);
        QPolygonF points = inkStroke.points();
        points = transform.map(points);
        ObjectInkStroke resInkStroke(inkStroke.color(), points, inkStroke.penSize());
        result.push_back(resInkStroke.toJson());
    }

    return result;
}

QRectF InkManger::normalizeRect(QSharedPointer<StageItem> stageItem)
{
    auto size = stageItem->metadata()->geometry().size();
    return QRectF(0, 0, 1, size.height() * 1.0 / size.width());
}

QRectF InkManger::normalBounds(QSharedPointer<StageItem> stageItem)
{
    StageItemMetadata* metadata = stageItem->metadata().data();
    QJsonArray inkStrokes = metadata->inkStrokes();
    if(inkStrokes.empty())
        return QRectF();

    return normalBounds(applyPerspective(inkStrokes, stageItem), stageItem);
}

QRectF InkManger::normalBounds(const QJsonArray &strokes, QSharedPointer<StageItem> stageItem)
{
    auto size = stageItem->metadata()->geometry().size();
    QRectF imgRect(0, 0, 1, size.height() * 1.0 / size.width());

    QRectF bounds(imgRect);
    for(auto strokeJson : strokes)
    {
        QJsonObject strokeObj = strokeJson.toObject();
        ObjectInkStroke inkStroke(strokeObj);
        bounds = bounds.united(inkStroke.bounds());
    }

    return bounds;
}

} // namespace common
} // namespace capture
