/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "view.h"

#ifndef QT_NO_OPENGL
#include <QtOpenGL>
#else
#include <QtWidgets>
#endif
#include <qmath.h>

MyGraphicsView::MyGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    view = new View("");
    this->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    this->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
}

#ifndef QT_NO_WHEELEVENT
void MyGraphicsView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->delta() > 0)
            view->zoomIn(6);
        else
            view->zoomOut(6);
        e->accept();
    } else {
        QGraphicsView::wheelEvent(e);
    }
}
#endif

View::View(const QString &name, QWidget *parent)
    : QFrame(parent)
{
    setStyleSheet("QFrame {  border: 2px solid #7d7d7d; }");

    setFrameStyle(Sunken | StyledPanel);
//    graphicsView->setRenderHint(QPainter::Antialiasing, false);
//    graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
//    graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
//    graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
//    graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
//    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
//    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff );

    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QSize iconSize(size, size);

    QToolButton *zoomInIcon = new QToolButton;
    zoomInIcon->setAutoRepeat(true);
    zoomInIcon->setAutoRepeatInterval(33);
    zoomInIcon->setAutoRepeatDelay(0);
    zoomInIcon->setIcon(QPixmap(":/zoomin.png"));
    zoomInIcon->setIconSize(iconSize);
    QToolButton *zoomOutIcon = new QToolButton;
    zoomOutIcon->setAutoRepeat(true);
    zoomOutIcon->setAutoRepeatInterval(33);
    zoomOutIcon->setAutoRepeatDelay(0);
    zoomOutIcon->setIcon(QPixmap(":/zoomout.png"));
    zoomOutIcon->setIconSize(iconSize);

    // Zoom slider layout
    QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
    zoomSliderLayout->addWidget(zoomInIcon);
    zoomSliderLayout->addWidget(zoomOutIcon);

    QToolButton *rotateLeftIcon = new QToolButton;
    rotateLeftIcon->setIcon(QPixmap(":/rotateleft.png"));
    rotateLeftIcon->setIconSize(iconSize);
    QToolButton *rotateRightIcon = new QToolButton;
    rotateRightIcon->setIcon(QPixmap(":/rotateright.png"));
    rotateRightIcon->setIconSize(iconSize);
    rotateSlider = new QSlider;
    rotateSlider->setOrientation(Qt::Horizontal);
    rotateSlider->setMinimum(-360);
    rotateSlider->setMaximum(360);
    rotateSlider->setValue(0);
    rotateSlider->setTickPosition(QSlider::TicksBelow);

    // Rotate slider layout
    QHBoxLayout *rotateSliderLayout = new QHBoxLayout;
    rotateSliderLayout->addWidget(rotateLeftIcon);
    rotateSliderLayout->addWidget(rotateSlider);
    rotateSliderLayout->addWidget(rotateRightIcon);

    resetButton = new QToolButton;
    resetButton->setText(tr("0"));
    resetButton->setEnabled(false);

    // Label layout
    QHBoxLayout *labelLayout = new QHBoxLayout;
    selectModeButton = new QCheckBox;
    selectModeButton->setText(tr("Select"));
    selectModeButton->setCheckable(true);
    selectModeButton->setChecked(false);
    openGlButton = new QToolButton;
    openGlButton->setText(tr("OpenGL"));
    openGlButton->setCheckable(true);
#ifndef QT_NO_OPENGL
    openGlButton->setEnabled(QGLFormat::hasOpenGL());
#else
    openGlButton->setEnabled(false);
#endif

    labelLayout->addWidget(selectModeButton);
    labelLayout->addStretch();
    labelLayout->addWidget(openGlButton);

    QGridLayout *topLayout = new QGridLayout;
    topLayout->addLayout(labelLayout, 0, 0);
    //topLayout->addWidget(graphicsView, 1, 0);
    topLayout->addLayout(zoomSliderLayout, 1, 1);
    topLayout->addLayout(rotateSliderLayout, 2, 0);
    topLayout->addWidget(resetButton, 2, 1);
    setLayout(topLayout);

    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetView()));
    connect(rotateSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    connect(selectModeButton, SIGNAL(toggled(bool)), this, SLOT(togglePointerMode()));
    connect(openGlButton, SIGNAL(toggled(bool)), this, SLOT(toggleOpenGL()));
    connect(rotateLeftIcon, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    connect(rotateRightIcon, SIGNAL(clicked()), this, SLOT(rotateRight()));
    connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));

    setupMatrix();
}

//QGraphicsView *View::view() const
//{
//    return static_cast<QGraphicsView *>(graphicsView);
//}

void View::resetView()
{
    rotateSlider->setValue(0);
    setupMatrix();
    //ensureVisible(QRectF(0, 0, 0, 0));

    resetButton->setEnabled(false);
}

void View::setResetButtonEnabled()
{
    resetButton->setEnabled(true);
}

void View::setupMatrix()
{
}

void View::togglePointerMode()
{
//    graphicsView->setDragMode(selectModeButton->isChecked()
//                              ? QGraphicsView::RubberBandDrag
//                              : QGraphicsView::ScrollHandDrag);
//    graphicsView->setInteractive(selectModeButton->isChecked());
}

void View::toggleOpenGL()
{
#ifndef QT_NO_OPENGL
    //graphicsView->setViewport(openGlButton->isChecked() ? new QOpenGLWidget() : new QWidget);
#endif
}

void View::toggleAntialiasing()
{
    //graphicsView->setRenderHint(QPainter::Antialiasing, antialiasButton->isChecked());
}

void View::zoomIn(int level)
{
    //zoomSlider->setValue(zoomSlider->value() + level);
}

void View::zoomOut(int level)
{
    //zoomSlider->setValue(zoomSlider->value() - level);
}

void View::rotateLeft()
{
    rotateSlider->setValue(rotateSlider->value() - 10);
}

void View::rotateRight()
{
    rotateSlider->setValue(rotateSlider->value() + 10);
}
