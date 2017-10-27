
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
#include "presentation_window.h"
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
    m_webCam_item->setPos(1000, 500);
    m_scene->addItem(m_webCam_item);

    m_monitor_item = new DownCam(QSize(600, 340));
    m_monitor_item->setPos(200, 500);
    m_scene->addItem(m_monitor_item);

    ui->graphicsView->setScene(m_scene);
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

    QTimer::singleShot(100, this, [this] {
        m_downCam >> m_frame_downCam;
        //cv::resize(m_frame_downCam, m_frame_downCam, cv::Size(width(), height()));
        m_image_downCam = Mat2QImage(m_frame_downCam);
        //m_downCam_item->setSize(width(), height());
        m_downCam_item->setImage(m_image_downCam);
    });

    QTimer::singleShot(100, this, [this] {
        m_webCam >> m_frame_webCam;
        m_image_webCam = Mat2QImage(m_frame_webCam);
        m_webCam_item->setImage(m_image_webCam);
    });

    QTimer::singleShot(100, this, [this] {
        HWND hwndDesktop = GetDesktopWindow();
        m_frame_monitor = hwnd2mat(hwndDesktop);
        m_image_monitor = Mat2QImage(m_frame_monitor);
        m_monitor_item->setImage(m_image_monitor);
    });
}

cv::Mat PresentMainWindow::hwnd2mat(HWND hwnd)
{
    HDC hwindowDC, hwindowCompatibleDC;

    int height, width, srcheight, srcwidth;
    HBITMAP hbwindow;
    Mat src;
    BITMAPINFOHEADER  bi;

    hwindowDC = GetDC(hwnd);
    hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    RECT windowsize;    // get the height and width of the screen
    GetClientRect(hwnd, &windowsize);

    srcheight = windowsize.bottom;
    srcwidth = windowsize.right;
    height = windowsize.bottom / 1;  //change this to whatever size you want to resize to
    width = windowsize.right / 1;

    src.create(height, width, CV_8UC4);

    // create a bitmap
    hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);
    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow
                                                                                                       // avoid memory leak
    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

    return src;
}

/*
int main()
{
    if (!cv::ocl::haveOpenCL())
    {
        cout << "OpenCL is not avaiable..." << endl;
        return 0;
    }
    cv::ocl::Context context;
    if (!context.create(cv::ocl::Device::TYPE_GPU))
    {
        cout << "Failed creating the context..." << endl;
        return 0;
    }

    // In OpenCV 3.0.0 beta, only a single device is detected.
    cout << context.ndevices() << " GPU devices are detected." << endl;
    for (int i = 0; i < context.ndevices(); i++)
    {
        cv::ocl::Device device = context.device(i);
        cout << "name                 : " << device.name() << endl;
        cout << "available            : " << device.available() << endl;
        cout << "imageSupport         : " << device.imageSupport() << endl;
        cout << "OpenCL_C_Version     : " << device.OpenCL_C_Version() << endl;
        cout << endl;
    }

    // Select the first device
    cv::ocl::Device(context.device(0));

    cv::ocl::setUseOpenCL(true);

    VideoCapture cap1;
    VideoCapture cap2;
    //cap1.open(1);
    //cap2.open(2);

    cap1.open(CV_CAP_DSHOW + 0);
    cap2.open(CV_CAP_DSHOW + 1);

    if (!cap1.isOpened() )//|| !cap2.isOpened())
    {
        return -1;
    }

    cap1.set(CV_CAP_PROP_FRAME_WIDTH, 4416);
    cap1.set(CV_CAP_PROP_FRAME_HEIGHT, 3312);

    namedWindow("Video", 1);
    namedWindow("Video", 2);
    while (1)
    {
        //Mat frame1;
        UMat frame1;
        //cv::cuda::GpuMat frame1;
        double start_time = cv::getTickCount();
        cout << "Start:" << start_time << endl;
        cap1 >> frame1;
        cout << "End:" << double(cv::getTickCount()) << endl;
        //cout << getTickCount() - start_time <<endl;
        int nHeight = frame1.rows;
        int nWidth = frame1.cols;
        //resize(image, image, Size(640, 360), 0, 0, INTER_CUBIC);
        cv::resize(frame1, frame1, cv::Size(1000, 600));
        imshow("Video1", frame1);
        waitKey(1);

        UMat frame2;
        cap2 >> frame2;
        imshow("Video2", frame2);
        waitKey(1);

        HWND hwndDesktop = GetDesktopWindow();
        Mat frame3 = hwnd2mat(hwndDesktop);
        cv::resize(frame3, frame3, cv::Size(888, 500));
        imshow("Video3", frame3);
        waitKey(1);
    }
    cap1.release();
    cap2.release();
    return 0;
}
*/


/* Convert  cv::Mat to QImage without data copy
 */
QImage PresentMainWindow::mat2Image_shared(const cv::Mat &mat, QImage::Format formatHint)
{
    Q_ASSERT(mat.type() == CV_8UC1 || mat.type() == CV_8UC3 || mat.type() == CV_8UC4);

    if (mat.empty())
        return QImage();

    //Adjust formatHint if needed.
    if (mat.type() == CV_8UC1) {
        if (formatHint != QImage::Format_Indexed8
        #if QT_VERSION >= 0x050500
                && formatHint != QImage::Format_Alpha8
                && formatHint != QImage::Format_Grayscale8
        #endif
                ) {
            formatHint = QImage::Format_Indexed8;
        }
#if QT_VERSION >= 0x040400
    } else if (mat.type() == CV_8UC3) {
        formatHint = QImage::Format_RGB888;
#endif
    } else if (mat.type() == CV_8UC4) {
        if (formatHint != QImage::Format_RGB32
                && formatHint != QImage::Format_ARGB32
                && formatHint != QImage::Format_ARGB32_Premultiplied
        #if QT_VERSION >= 0x050200
                && formatHint != QImage::Format_RGBX8888
                && formatHint != QImage::Format_RGBA8888
                && formatHint != QImage::Format_RGBA8888_Premultiplied
        #endif
                ) {
            formatHint = QImage::Format_ARGB32;
        }
    }

    QImage img(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), formatHint);

    //Should we add directly support for user-customed-colorTable?
    if (formatHint == QImage::Format_Indexed8) {
        QVector<QRgb> colorTable;
        for (int i=0; i<256; ++i)
            colorTable.append(qRgb(i,i,i));
        img.setColorTable(colorTable);
    }
    return img;
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

QImage PresentMainWindow::Mat2QImage(cv::Mat const& src)
{
    cv::Mat temp; // make the same cv::Mat
    cvtColor(src, temp, CV_BGR2RGB); // cvtColor Makes a copy, that what i need
    QImage dest((const uchar *)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    dest.bits(); // enforce deep copy, see documentation 
                 // of QImage::QImage ( const uchar * data, int width, int height, Format format )
    return dest;
}