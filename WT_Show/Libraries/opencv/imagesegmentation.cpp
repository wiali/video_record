#include "imagesegmentation.h"
#include "cvmatandqimage.h"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;

//*****************************************************************************
int ImageSegmentation::GetImages(QImage input, QList<QImage>& output, QList<QPoint>& positions)
{
	Mat image = image2Mat(input);
	if (image.rows == 0 && image.cols == 0)
		return -1; //no image

	Mat frame(image);
	//proceed frame for easier detection
	cvtColor(frame, frame, CV_BGR2GRAY);
	blur(frame, frame, Size(3, 3));
    threshold(frame, frame, 0, 255, THRESH_OTSU); //THRESH_BINARY_INV ? need to improve

    // detect edges using canny
    Mat canny_output;
    Canny( frame, canny_output, 100, 150, 5, true );

    dilate(canny_output,canny_output,Mat());
    erode(canny_output,canny_output,Mat());

	//find contours
    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	if (!contours.size())
		return -2; //no contours

	//find items by contours
	for (size_t i = 0; i < contours.size(); i++)
	{
		Rect bounds = boundingRect(contours.at(i));
		if (bounds.width < 50 || bounds.height < 50)
			continue;

		Mat mask = Mat::zeros(frame.rows, frame.cols, CV_8UC1);
		drawContours(mask, contours, (int) i, Scalar(255), CV_FILLED);
		Mat crop(image.rows, image.cols, CV_8UC4);
		crop.setTo(Scalar(0, 0, 0, 0));
		image.copyTo(crop, mask);
		Mat result(crop, bounds);
		positions.append(QPoint(bounds.x, bounds.y));
		output.append(mat2Image(result));
	}
	if (output.empty())
		return -3; //empty list
	return 0;
}
