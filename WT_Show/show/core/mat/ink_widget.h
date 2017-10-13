/*! \file ink_tool_box.h
 *  \brief Provided a ink canvas used for drawing with mouse, touch or stencil
 *  \details This class is based on Qt official example: http://doc.qt.io/qt-5/qtwidgets-widgets-tablet-tabletcanvas-h.html
 *  \date February, 2017
 *  \copyright 2017 HP Development Company, L.P.
 */
#pragma once
#ifndef INK_TOOL_BOX_H
#define INK_TOOL_BOX_H

#include <QWidget>
#include <QTimer>
#include <QColor>
#include <QSharedPointer>
#include <QPixmap>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#include <frame_counting.h>

#include "ink_data.h"
#include "shared_global.h"
#include "model/application_state_model.h"
#include "stage_item_3d.h"

namespace capture {
namespace mat {

class SHAREDSHARED_EXPORT InkWidget : public QWidget
{
    Q_OBJECT

public:
    InkWidget(QWidget* leftWidget, QWidget* rightWidget, QWidget *parent = nullptr, bool showFps=false, QSharedPointer<model::ApplicationStateModel> model = QSharedPointer<model::ApplicationStateModel>());

    /*! \brief Reset the Ink mode
     */
    void setPenMode(bool penMode);

    /*! \brief Reset the strokes data.
     *  Important!! The ink data must be set. Otherwise it will lost the ink data.
     */
    void setInkData(QSharedPointer<InkData> strokes);

    void setStageRenderer(StageRenderer* renderer);

    /*! \brief Reset the pen color
     */
    void setColor(const QColor &c);

    /*! \brief Get the pen color
     */
    QColor color() const;

    /*! \brief Reset the pen point color
     */
    void setPenPointColor(const QColor &penPointColor);

    /*! \brief Get the pen point color
     */
    QColor penPointColor() const;

    /*! \brief Enter eraser mode
     */
	void enterEraserMode();

    /*! \brief Enable pen
     */
    void enablePen(bool enable);

    /*! \brief Enter eraser mode
     */
    void enableRemoveStroke(bool enable);

    /*! \brief Enter drawing mode
     */
	void enterDrawMode();

    void setForward(bool forward) { m_forward = forward; }

    bool forward() { return m_forward; }

    void updatePixmapBySize();

    void onUpdate();

public slots:
    /*! \brief Handles the resize of our main window
     */
    void mainWindowResize(QSize size);

    /*! \brief Resizes the draing pixmap and overlay to the parent window
     */
    void resizeToWindow();

    /*! \brief Reset the pen size
     */
	void penSizeChanged(int penSize);

    /*! \brief Reset the pen color
     */
	void colorChanged(const QColor& color);

    /*! \brief Digitizer pen enters hovering
     */
    void onPenHoverEntered(const POINTER_PEN_INFO& penInfo);

    /*! \brief Digitizer pen exits hovering
     */
    void onPenHoverExited(const POINTER_PEN_INFO& penInfo);

    /*! \brief Digitizer pen touch touchmat
     */
    void onPenDown(const POINTER_PEN_INFO& penInfo);

    /*! \brief Digitizer pen leave touchmat.
     */
    void onPenUp(const POINTER_PEN_INFO& penInfo);

    /*! \brief Digitizer pen move on touchmat
     */
    void onPenMove(const POINTER_PEN_INFO& penInfo);

    void updatePixmap(bool updateAll);

signals:

    /*! \brief Ink data has been changed.
     */
    void inkDataChanged(QSharedPointer<InkData> inkData);

    /*! \brief Ink data has been erased
     */
    void inkDataErasing(const QPoint& point);

    /*! \brief Ink point have been added.
     */
    void inkPointAdded(const QPoint& point, double width);

    /*! \brief Ink stroke have been added.
     */
    void inkStrokeAdded();

protected:
    /*! \brief Paint event
     */
    void paintEvent(QPaintEvent *event) override;

    /*! \brief Mouse move event
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /*! \brief Mouse press event
     */
    void mousePressEvent(QMouseEvent *event) override;

    /*! \brief Mouse release event
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /*! \brief Hide event
     */
    void hideEvent(QHideEvent *event) override;

    /*! \brief Show event
     */
    void showEvent(QShowEvent *event) override;

private slots:
    void onEditModeChanged(model::ApplicationStateModel::EditMenuMode mode);

private:
	void changeCursor(int penSize);
    QString penInfoStr(const POINTER_PEN_INFO& penInfo);

    // Erase the stroke that near the point
    void eraseStroke(const QPoint& pos);

    // Update the pen color
	void updateColor(const QColor& color);

    // Add point to current stroke.
    void addPoint(const QPoint& point, double width);

    // Add current stroke to the list
    void addStroke();

    QPolygonF corner();

    QColor m_color;
    int m_basePenWidth;

    /*! \brief Timer used to emulate the end of a resize
     */
    QTimer m_resizeTimer;
	
    // Eraser size
	int m_eraserSize;

    // Right menu widget
    QWidget* m_rightMenu;

    // Left menu widget
    QWidget* m_leftMenu;

    // Eraser mode or not
    bool m_eraserMode;
    bool m_penEraserMode;

    // Pen mode or not
    bool m_penMode;

    // Pen is drawing. And we will discard the mouse event.
    bool m_penDrawing;

    // Enable pen
    bool m_enablePen;

    // Mouse is drawing
    bool m_mouseDrawing;

    // Disable the remove feature
    bool m_enableRemoveStroke;

    bool m_enabled;

    QColor m_penPointColor;

    // Ink data
    QSharedPointer<InkData> m_inkData;

    // Shadow image to improve the render performance.
    QPixmap m_pixmap;

    QSharedPointer<FrameCounting> m_frameCounting;

    bool m_forward = false;

    QSharedPointer<model::ApplicationStateModel> m_model;

    StageRenderer* m_renderer;
};

} // namespace mat
} // namespace capture

#endif // INK_TOOL_BOX_H
