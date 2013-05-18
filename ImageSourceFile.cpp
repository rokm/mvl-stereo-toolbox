#include "ImageSourceFile.h"

#include <opencv2/highgui/highgui.hpp>


ImageSourceFile::ImageSourceFile (QObject *parent)
    : ImageSource(parent)
{
    shortName = "FILE";
    configWidget = NULL;
}

ImageSourceFile::~ImageSourceFile ()
{
}

void ImageSourceFile::loadImagePair (const QString &filenameLeft, const QString &filenameRight)
{
    imageLeft = cv::imread(filenameLeft.toStdString());
    imageRight = cv::imread(filenameRight.toStdString());

    emit imagesChanged();
}
