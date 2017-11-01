
#include "chip.h"
#include "presentation_mainwindow.h"
#include "view.h"

#include <QString>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QScreen>
#include <QTimer>

#include "ui_presentation_mainwindow.h"
#include "video_widget.h"
#include "down_cam.h"

/*
QGraphicsEllipseItem  提供一个椭圆item
QGraphicsLineItem     提供一条线的item
QGraphicsPathItem     提供一个任意的路径item
QGraphicsPixmapItem   提供一个图形item
QGraphicsPolygonItem  提供一个多边形item
QGraphicsRectItem     提供一个矩形item
QGraphicsSimpleTextItem 提供一个简单的文本item
QGraphicsTextItem     提供一个文本浏览item
*/
const QString MONITOR = "monitor";
const QString MAT = "mat";
const QString PRESENT = "present";

QStringList PresentMainWindow::m_matHardwareIds = { "HWP4239" };
QStringList PresentMainWindow::m_monitorHardwareIds = { "HWP4627", "HWP425D" };

static QHash<ScreenType, QString> ScreenTypeTranslationTable{
    { ScreenType::MonitorScreen, MONITOR },
    { ScreenType::MatScreen, MAT },
    { ScreenType::PresentScreen, PRESENT } };


using namespace std;
using namespace cv;

PresentMainWindow::PresentMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PresentMainWindow)
{

    ui->setupUi(this);

    setAutoFillBackground(false);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground);

    setWindowTitle(tr("Presentation Window"));

    m_scene = new QGraphicsScene; 

    m_downCam.open(CV_CAP_DSHOW + 0);
    if (!m_downCam.isOpened() )
    {
        return;
    }
    m_downCam.set(CV_CAP_PROP_FRAME_WIDTH, 4416);
    m_downCam.set(CV_CAP_PROP_FRAME_HEIGHT, 3312);
    m_downCam_item = new DownCam(QSize(1920,1080));
    m_downCam_item->setPos(0, 0);
    m_scene->addItem(m_downCam_item);

    m_webCam.open(CV_CAP_DSHOW + 1);
    if (!m_webCam.isOpened())
    {
        return;
    }
    m_webCam_item = new DownCam(QSize(600, 340));
    m_webCam_item->setPos(1000, 600);
    m_scene->addItem(m_webCam_item);

    m_mat_item = new DownCam(QSize(600, 340));
    m_mat_item->setPos(200, 100);
    m_scene->addItem(m_mat_item);

    m_monitor_item = new DownCam(QSize(600, 340));
    m_monitor_item->setPos(200, 600);
    m_scene->addItem(m_monitor_item);

    ui->graphicsView->setScene(m_scene);

    m_matScreen = findScreen(MatScreen);
    m_monitorScreen = findScreen(MonitorScreen);

    double coefficients[3][3] = { 1 };
    QTransform invertedHomography = QTransform(
        0.99800744125081942,
        -0.12494066354552548,
        -58.492698385016908,
        0.0031995645556478947,
        0.96248397740829172,
        -333.29982033723468,
        2.3323995676436789e-07,
        -5.8578188344702079e-05,
        1);

    invertedHomography = invertedHomography.inverted();
    coefficients[0][0] = invertedHomography.m11();
    coefficients[0][1] = invertedHomography.m12();
    coefficients[0][2] = invertedHomography.m13();
    coefficients[1][0] = invertedHomography.m21();
    coefficients[1][1] = invertedHomography.m22();
    coefficients[1][2] = invertedHomography.m23();
    coefficients[2][0] = invertedHomography.m31();
    coefficients[2][1] = invertedHomography.m32();
    coefficients[2][2] = invertedHomography.m33();

    m_keyStone_matrix = cv::Mat(3, 3, CV_64F, coefficients).inv();
    m_correctedSize = cv::Size(4200, 2800);
}

PresentMainWindow::~PresentMainWindow()
{
    m_downCam.release();
    delete ui;
}

void PresentMainWindow::paintEvent(QPaintEvent *event)
{
    if (!m_downCam.isOpened() )
        return;

    QTimer::singleShot(40, this, [this] {
        m_downCam >> m_frame_downCam;

        cv::Mat frame_keyStone_downCam;
        warpPerspective(m_frame_downCam, frame_keyStone_downCam, m_keyStone_matrix, m_correctedSize);
        //m_image_downCam = cvMatToQImage(m_frame_downCam);
        m_image_downCam = cvMatToQImage(frame_keyStone_downCam);
        m_downCam_item->setImage(m_image_downCam);
    });

    QTimer::singleShot(40, this, [this] {
        m_webCam >> m_frame_webCam;
        m_image_webCam = cvMatToQImage(m_frame_webCam);
        m_webCam_item->setImage(m_image_webCam);
    });

    QTimer::singleShot(40, this, [this] {
        QRect g = m_matScreen->geometry();
        m_image_mat = m_matScreen->grabWindow(0, g.x(), g.y(), g.width(), g.height()).toImage();
        m_mat_item->setImage(m_image_mat);
    });

    QTimer::singleShot(40, this, [this] {
        QRect g = m_monitorScreen->geometry();
        m_image_monitor = m_monitorScreen->grabWindow(0, g.x(), g.y(), g.width(), g.height()).toImage();
        m_monitor_item->setImage(m_image_monitor);
    });
}

QScreen* PresentMainWindow::findScreen(const ScreenType& type, int* index)
{
    QScreen* result = nullptr;
    QMap<QString, QString> map = getScreenNames();

    qInfo() << "The monitor screen name is" << map[MONITOR];
    qInfo() << "The mat screen name is" << map[MAT];
    qInfo() << "The present screen name is" << map[PRESENT];

    QList<QScreen*> screens = QGuiApplication::screens();

    int i = 0;
    QString key = ScreenTypeTranslationTable[type];
    foreach(auto screen, screens)
    {
        if (map.contains(key))
        {
            QString xxx = screen->name();
            if (screen->name() == map[key])
            {
                if (index)
                {
                    *index = i;
                }
                result = screen;
                return result;
            }
        }
        i++;
    }

    return result;
}

void PresentMainWindow::setHardwareIds(QStringList monitorHardwareIds, QStringList matHardwareIds) {
    m_monitorHardwareIds = monitorHardwareIds;
    m_matHardwareIds = matHardwareIds;
}

//[hardware]
//.mat_ids = HWP4239
//.monitor_ids = HWP4627, HWP425D

QRect PresentMainWindow::findScreenGeometry(const ScreenType& type)
{
#ifdef Q_OS_WIN
    DISPLAY_DEVICE dd;

    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);

    for (DWORD i = 0;; i++) {
        if (EnumDisplayDevices(NULL, i, &dd, 0) == 0) {
            break;
        }

        DISPLAY_DEVICE monInfo;
        monInfo.cb = sizeof(monInfo);

        for (DWORD j = 0; EnumDisplayDevices(dd.DeviceName, j, &monInfo, 0); j++)  // query all monitors on the adaptor
        {
            auto deviceId = QString::fromWCharArray(monInfo.DeviceID);
            auto deviceName = QString::fromWCharArray(dd.DeviceName);

            qDebug() << "Found device hardware ID" << deviceId << "as" << deviceName;

            DEVMODE defaultMode;

            ZeroMemory(&defaultMode, sizeof(DEVMODE));
            defaultMode.dmSize = sizeof(DEVMODE);

            if (!EnumDisplaySettings(dd.DeviceName, ENUM_REGISTRY_SETTINGS, &defaultMode))
            {
                qCritical() << "Can not enumerate the monitor setting!";
                break;
            }

            QRect geometry = QRect(defaultMode.dmPosition.x, defaultMode.dmPosition.y,
                defaultMode.dmPelsWidth, defaultMode.dmPelsHeight);

            bool bPresent = true;
            foreach(auto hardwareId, m_monitorHardwareIds) {
                if (deviceId.contains(hardwareId)) {
                    bPresent = false;
                    if (type == ScreenType::MonitorScreen)
                    {
                        qInfo() << "Find Monitor screen, the geometry is" << geometry;
                        return geometry;
                    }
                    break;
                }
            }

            foreach(auto hardwareId, m_matHardwareIds) {
                if (deviceId.contains(hardwareId)) {
                    bPresent = false;
                    if (type == ScreenType::MatScreen)
                    {
                        qInfo() << "Find Mat screen, the geometry is" << geometry;
                        return geometry;
                    }
                    break;
                }
            }

            if (type == ScreenType::PresentScreen && bPresent)
            {
                qInfo() << "Find Present screen, the geometry is" << geometry;
                return geometry;
            }
        }
    }

#elif
#error Not implemented for this OS
#endif

    return QRect();
}

QMap<QString, QString> PresentMainWindow::getScreenNames()
{
    QMap<QString, QString> screenMap;
#ifdef Q_OS_WIN
    DISPLAY_DEVICE dd;

    ZeroMemory(&dd, sizeof(dd));
    dd.cb = sizeof(dd);

    for (DWORD i = 0;; i++) {
        if (EnumDisplayDevices(NULL, i, &dd, 0) == 0) {
            break;
        }

        DISPLAY_DEVICE monInfo;
        monInfo.cb = sizeof(monInfo);

        for (DWORD j = 0; EnumDisplayDevices(dd.DeviceName, j, &monInfo, 0); j++)  // query all monitors on the adaptor
        {
            auto deviceId = QString::fromWCharArray(monInfo.DeviceID);
            auto deviceName = QString::fromWCharArray(dd.DeviceName);

            qDebug() << "Found device hardware ID" << deviceId << "as" << deviceName;

            foreach(auto hardwareId, m_monitorHardwareIds) {
                if (deviceId.contains(hardwareId)) {
                    screenMap.insert(MONITOR, deviceName);
                    break;
                }
            }

            foreach(auto hardwareId, m_matHardwareIds) {
                if (deviceId.contains(hardwareId)) {
                    screenMap.insert(MAT, deviceName);
                    break;
                }
            }

            if (screenMap.contains(MONITOR) && deviceName != screenMap[MONITOR] &&
                screenMap.contains(MAT) && deviceName != screenMap[MAT]) {
                screenMap.insert(PRESENT, deviceName);
            }
        }
    }

#elif
#error Not implemented for this OS
#endif

    return screenMap;
}

QImage PresentMainWindow::cvMatToQImage(const cv::Mat& inMat)
{
    switch (inMat.type())
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image(inMat.data,
            inMat.cols, inMat.rows,
            static_cast<int>(inMat.step),
            QImage::Format_ARGB32);

        return image;
    }

    // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image(inMat.data, inMat.cols, inMat.rows,
            static_cast<int>(inMat.step), QImage::Format_RGB888);

        return image.rgbSwapped();
    }

    // 8-bit, 1 channel
    case CV_8UC1:
    {
        QImage image(inMat.data,
            inMat.cols, inMat.rows,
            static_cast<int>(inMat.step),
            QImage::Format_Grayscale8);

        return image;
    }

    default:
        qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
        break;
    }

    return QImage();
}
