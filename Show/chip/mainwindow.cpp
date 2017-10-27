
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
#include <QThread>
#include <QTimer>

#include "ui_mainwindow.h"
#include "video_widget.h"
#include "presentation_mainwindow.h"
#include "down_cam.h"


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

    setGeometry(1645, 100, width(), height());

    scene = new QGraphicsScene;
    QGraphicsView* view = new QGraphicsView(scene);    

    int itemPos[] = { 0,130,260,390 };
    int textPos[] = {45, 35, 45, 35};
    QString texts[] = {"Monitor", "Touch Mat", "WebCam", "Down Cam"};

    for(int i = 0; i < 4; i++)
    {
        DownCam* pCamItem = new DownCam(QSize(164, 90));
        m_Cam_items.push_back(pCamItem);
        pCamItem->setPos(0, itemPos[i]);
        scene->addItem(pCamItem);

        QGraphicsTextItem* mTextItem = new QGraphicsTextItem(texts[i]);
        //space 40
        mTextItem->setDefaultTextColor(Qt::white);
        mTextItem->setFont(QFont("Segoe UI"));
        mTextItem->setPos(textPos[i], itemPos[i] + pCamItem->boundingRect().height()+2);
        scene->addItem(mTextItem);
    }

    view->setScene(scene);

    ui->verticalLayout->addWidget(view, 0);

    connect(ui->PresentButton, SIGNAL(clicked()), this, SLOT(onPresentation()));

    setWindowTitle(tr("WorkTools Show"));

    m_window = new PresentMainWindow();
    m_window->setVisible(false);
}

void MainWindow::onPresentation()
{
//    VideoWidget vw;
//    vw.show();

    QRect geometry = PresentMainWindow::findScreenGeometry(PresentScreen);
    if (!geometry.isNull()) 
    {
        m_window->setGeometry(geometry);
        m_window->setVisible(true);
        m_window->showMaximized();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::paintEvent(QPaintEvent *event)
{
    if (!m_window->isVisible())
    {
        QMainWindow::paintEvent(event);
        return;
    }

    QTimer::singleShot(100, this, [this] {
        m_Cam_items[0]->setImage(m_window->m_image_monitor);
    });

    QTimer::singleShot(100, this, [this] {
        m_Cam_items[2]->setImage(m_window->m_image_webCam);
    });

    QTimer::singleShot(100, this, [this] {
        m_Cam_items[3]->setImage(m_window->m_image_downCam);
    });

    QMainWindow::paintEvent(event);
}