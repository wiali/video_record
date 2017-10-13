#pragma once
#ifndef INK_MANGER_H
#define INK_MANGER_H

#include <QObject>

#include "stage_item.h"
#include "ink_stroke.h"
#include "object_ink_stroke.h"

namespace capture {
namespace common {

class InkManger : public QObject
{
    Q_OBJECT
public:
    explicit InkManger(QObject *parent = nullptr);

    static void addStroke(QSharedPointer<InkStroke> stroke, QSharedPointer<StageItem> stageItem, const QPolygonF& corners);

    static QSizeF perspectiveSize(const QPolygonF &perspectiveQuad);
    static ObjectInkStroke convertInkStroke(QSharedPointer<InkStroke> inkStroke,QSharedPointer<StageItem> stageItem, const QPolygonF& corners);
    static QJsonArray applyPerspective(const QJsonArray &strokes, QSharedPointer<StageItem> stageItem);
    static QRectF normalizeRect(QSharedPointer<StageItem> stageItem);
    static QRectF normalBounds(QSharedPointer<StageItem> stageItem);
    static QRectF normalBounds(const QJsonArray &strokes, QSharedPointer<StageItem> stageItem);

signals:

public slots:
};

} // namespace common
} // namespace capture

#endif // INK_MANGER_H
