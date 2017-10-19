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

#include "chip.h"
#include "mainwindow.h"
#include "view.h"

#include <QString>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

#include "ui_mainwindow.h"
#include "video_widget.h"
#include "presentation_window.h"

/*
 * QGraphicsEllipseItem  提供一个椭圆item
QGraphicsLineItem     提供一条线的item
QGraphicsPathItem     提供一个任意的路径item
QGraphicsPixmapItem   提供一个图形item
QGraphicsPolygonItem  提供一个多边形item
QGraphicsRectItem     提供一个矩形item
QGraphicsSimpleTextItem 提供一个简单的文本item
QGraphicsTextItem     提供一个文本浏览item
*/

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    setGeometry(1645, 100, 275, 820);

    QHBoxLayout *topLayout = new QHBoxLayout;
    QToolButton *zoomInIcon = new QToolButton;
    zoomInIcon->setIcon(QPixmap(":/zoomin.png"));
    zoomInIcon->setIconSize(QSize(32,32));
    topLayout->addWidget(zoomInIcon);

    QToolButton *zoomOutIcon = new QToolButton;
    zoomOutIcon->setIcon(QPixmap(":/zoomout.png"));
    zoomOutIcon->setIconSize(QSize(32, 32));
    topLayout->addWidget(zoomOutIcon);
    topLayout->addStretch();


    QVBoxLayout *bottomLayout = new QVBoxLayout;

    QSpacerItem* space1 =new QSpacerItem(275, 20);
    bottomLayout->addSpacerItem(space1);

    bottomLayout->addWidget(new QLabel("Sources"), 0, Qt::AlignCenter);
    //bottomLayout->addStretch();

    QSpacerItem* space2 =new QSpacerItem(275, 10);
    bottomLayout->addSpacerItem(space2);

    scene = new QGraphicsScene;
    QGraphicsView* view = new QGraphicsView(scene);    

    int itemPos[] = { 0,130,260,390 };
    int textPos[] = {45, 35, 45, 35};
    QString texts[] = {"Monitor", "Touch Mat", "WebCam", "Down Cam"};

    for(int i = 0; i < 4; i++)
    {
        Chip *item = new Chip;

        item->setPos(0, itemPos[i]);
        scene->addItem(item);

        QGraphicsTextItem* mTextItem = new QGraphicsTextItem(texts[i]);
        //space 40
        mTextItem->setDefaultTextColor(Qt::white);
        mTextItem->setFont(QFont("Segoe UI"));
        mTextItem->setPos(textPos[i], itemPos[i] +item->boundingRect().height()+2);
        scene->addItem(mTextItem);
    }

    view->setScene(scene);

    bottomLayout->addWidget(view, 0, Qt::AlignCenter);    
    bottomLayout->addStretch();


    QToolButton *btn_present = new QToolButton;
    btn_present->setIcon(QPixmap(":/icon-present-press.png"));
    btn_present->setIconSize(QSize(64, 64));
    btn_present->setStyleSheet("background: transparent;");
    bottomLayout->addWidget(btn_present, 0, Qt::AlignCenter);
    bottomLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    ui->centralWidget->setLayout(mainLayout);

    ui->centralWidget->setStyleSheet("QWidget {"
        "font-family: Segoe UI;"
        "font-size: 20px;"
        "font-weight: normal;"
        "font-style: normal;"
        "text-align: center center;"
        "color: white;"
        "border-radius: 2px;"
        "background-color: #2d3338;"
        "}");

//    connect(buttonIcon, SIGNAL(clicked()), this, SLOT(onPresentation()));

    setWindowTitle(tr("WorkTools Show"));
}

void MainWindow::onPresentation()
{
//    VideoWidget vw;
//    vw.show();
    PresentationWindow window;
    window.show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

