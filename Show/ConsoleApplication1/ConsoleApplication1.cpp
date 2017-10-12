#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp> // for camera

#include "opencv2/imgproc.hpp"
#include <Windows.h>

using namespace cv;

Mat hwnd2mat(HWND hwnd)
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


int main()
{
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
        Mat frame1;
        cap1 >> frame1;
        int nHeight = frame1.rows;
        int nWidth = frame1.cols;
        //resize(image, image, Size(640, 360), 0, 0, INTER_CUBIC);
        cv::resize(frame1, frame1, cv::Size(1000, 600));
        imshow("Video1", frame1);
        waitKey(1);

        Mat frame2;
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