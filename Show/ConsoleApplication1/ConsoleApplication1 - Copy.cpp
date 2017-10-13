#include "stdafx.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp> // for camera
using namespace cv;

int main()
{
    VideoCapture cap1;
    VideoCapture cap2;
    //cap1.open(1);
    //cap2.open(2);

    cap1.open(0);

    if (!cap1.isOpened() )//|| !cap2.isOpened())
    {
        return -1;
    }
    namedWindow("Video", 1);
    //namedWindow("Video", 2);
    while (1)
    {
        Mat frame;
        cap1 >> frame;
        imshow("Video1", frame);
        waitKey(1);//用cv::waitKey来更新界面
        //cap2 >> frame;
        //imshow("Video2", frame);
        //waitKey(1);
    }
    cap1.release();
    cap2.release();
    return 0;
}