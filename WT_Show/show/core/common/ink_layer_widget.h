#pragma once
#ifndef INK_LAYER_WIDGET_H
#define INK_LAYER_WIDGET_H

#include <QWidget>
#include <QTransform>
#include "model/application_state_model.h"
#include "stage_renderer.h"

class InkData;
class FrameCounting;

namespace capture {
namespace common {

class InkLayerWidget : public QWidget
{
    Q_OBJECT

public:
    InkLayerWidget(QSharedPointer<model::ApplicationStateModel> model, QWidget *parent = nullptr);
    void setTransform(const QTransform& viewportMatrix);
    void setInkData(QSharedPointer<InkData> inkData);
    void setStageRenderer(StageRenderer* renderer);

protected:
    void paintEvent(QPaintEvent *event) override;

public slots:
    void updateCurrentInkData();
    void updateInkDisplay();
    void onUpdate();

private slots:
    void onGeometryChanged(const QRect& newRect);

    void connectCurrentStroke(QSharedPointer<InkStroke> stroke);
    void onStrokeAdded(QSharedPointer<InkStroke> addedStroke, QSharedPointer<InkStroke> currentStroke);

    void updateTransform();
    void onInkPixMapChanged(QPixmap inkPixMap);

private:
    QTransform m_viewportMatrix;
    QTransform m_inverseMatrix;

    QSharedPointer<FrameCounting> m_frameCounting;
    QSharedPointer<model::ApplicationStateModel> m_model;
    QSharedPointer<InkData> m_inkData;

    QVector<QMetaObject::Connection> m_inkDataConnections;
    QMetaObject::Connection m_inkStrokeConnection;

    StageRenderer* m_renderer;

    QPixmap m_inkPixMap;
};

} // namespace common
} // namespace capture

#endif // INK_LAYER_WIDGET_H
