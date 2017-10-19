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
#include "presentation_window.h"
#include "view.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <iostream>

using namespace std;
using namespace cv;

PresentationWindow::PresentationWindow(QWidget *parent)
    : QWidget(parent)
{
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

    populateScene();

//    View *view = new View("");
//    view->view()->setScene(scene);

//    view = new View("");
//    view->view()->setScene(scene);

//    view = new View("");
//    view->view()->setScene(scene);

//    view = new View("");
//    view->view()->setScene(scene);

    setWindowTitle(tr("Presentation"));
}

void PresentationWindow::populateScene()
{
    scene = new QGraphicsScene;

    QImage image(":/qt4logo.png");

    // Populate scene
    Chip *item = new Chip();
    item->setPos(QPointF(0, 0));
    scene->addItem(item);
}


cv::Mat PresentationWindow::hwnd2mat(HWND hwnd)
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
