#ifndef IMAGESEGMENTATION_H
#define IMAGESEGMENTATION_H
#include <opencv/cv.h>
#include <QImage>
#include <QList>
class ImageSegmentation
{
public:
	static int GetImages(QImage input, QList<QImage> &output, QList<QPoint> &positions);
};

#endif // IMAGESEGMENTATION_H
