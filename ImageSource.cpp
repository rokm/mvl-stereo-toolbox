#include "ImageSource.h"

ImageSource::ImageSource (QObject *parent)
    : QObject(parent)
{
}

ImageSource::~ImageSource ()
{
}

void ImageSource::getImages (cv::Mat &left, cv::Mat &right)
{
    // Copy images under lock, in case source implements dynamic image grab
    QReadLocker lock(&imagesLock);
    
    left = imageLeft;
    right = imageRight;
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
const QString &ImageSource::getShortName () const
{
    return shortName;
}

QWidget *ImageSource::getConfigWidget ()
{
    return configWidget;
}
