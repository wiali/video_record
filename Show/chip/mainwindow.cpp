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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

//    QHBoxLayout *layout = new QHBoxLayout;
//    layout->addWidget(toolBox);
//    view = new QGraphicsView(scene);
//    layout->addWidget(view);

//    QWidget *widget = new QWidget;
//    widget->setLayout(layout);

//    setCentralWidget(widget);

    ui->setupUi(this);

    setStyleSheet("QWidget {"
                  "font-family: Segoe UI;"
                  "font-size: 14px;"
                  "font-weight: normal;"
                  "font-style: normal;"
                  "text-align: center center;"
                  "color: white;"
                  "border-radius: 2px;"
                  "background-color: #2d3338;"
                  "}");

    scene = new QGraphicsScene;

    for(int i = 0; i < 4; i++)
    {
        QImage image(":/qt4logo.png");
        QColor color(image.pixel(int(image.width()), int(image.height())));
        Chip *item = new Chip(color, 0, 0);

        item->setPos(20, i*80+20);
        scene->addItem(item);

        QGraphicsTextItem* mTextItem = new QGraphicsTextItem("Monitor Screen");
        mTextItem->setPos(20, i*80+20 +item->boundingRect().height());
        scene->addItem(mTextItem);

        QGraphicsRectItem* item_space = new QGraphicsRectItem(
                    QRectF(20, i*80+20 +item->boundingRect().height() , item->boundingRect().width(), 40));
        item_space->setBrush(QBrush(QColor(Qt::green)));
        item_space->setOpacity(0.5);
        scene->addItem(item_space);
    }

    ui->myGraphicsView->setScene(scene);

//    QLabel* label1 = new QLabel(tr("Monitor"));
//    label1->setAlignment(Qt::AlignHCenter);

//    View *view = new View("");
//    view->view()->setScene(scene);


//    view = new View("");
//    view->view()->setScene(scene);
//    QWidget* spaceWindow2 = new QWidget;
//    QVBoxLayout *labelLayout2 = new QVBoxLayout;
//    QLabel* label2 = new QLabel(tr("TouchMat"));
//    label2->setAlignment(Qt::AlignHCenter);
//    labelLayout2->addWidget(view);
//    labelLayout2->addWidget(label2);
//    spaceWindow2->setLayout(labelLayout2);
//    vSplitter->addWidget(spaceWindow2);

//    view = new View("");
//    view->view()->setScene(scene);
//    QWidget* spaceWindow3 = new QWidget;
//    QVBoxLayout *labelLayout3 = new QVBoxLayout;
//    QLabel* label3 = new QLabel(tr("Web Cam"));
//    label3->setAlignment(Qt::AlignHCenter);
//    labelLayout3->addWidget(view);
//    labelLayout3->addWidget(label3);
//    spaceWindow3->setLayout(labelLayout3);
//    vSplitter->addWidget(spaceWindow3);

//    view = new View("");
//    view->view()->setScene(scene);
//    QWidget* spaceWindow4 = new QWidget;
//    QVBoxLayout *labelLayout4 = new QVBoxLayout;
//    QLabel* label4 = new QLabel(tr("Down Cam"));
//    label4->setAlignment(Qt::AlignHCenter);
//    labelLayout4->addWidget(view);
//    labelLayout4->addWidget(label4);
//    spaceWindow4->setLayout(labelLayout4);
//    vSplitter->addWidget(spaceWindow4);

//    QWidget* spaceWindow5 = new QWidget;
//    QVBoxLayout *labelLayout5 = new QVBoxLayout;
//    QToolButton *buttonIcon = new QToolButton;
//    buttonIcon->setIcon(QPixmap(":/icon-present-press.png"));
//    QSize iconSize(100, 100);
//    buttonIcon->setIconSize(iconSize);
//    //buttonIcon->setStyleSheet("font-size: 34px; color: white; background: transparent; font-family: Segoe UI;");
//    labelLayout5->addWidget(buttonIcon);
//    spaceWindow5->setLayout(labelLayout5);
//    labelLayout5->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//    vSplitter->addWidget(spaceWindow5);

//    QHBoxLayout *layout = new QHBoxLayout;
//    layout->addWidget(vSplitter);
//    setLayout(layout);

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

