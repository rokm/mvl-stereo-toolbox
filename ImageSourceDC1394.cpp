#include "ImageSourceDC1394.h"

ImageSourceDC1394::ImageSourceDC1394 (QObject *parent)
    : ImageSource(parent)
{
    shortName = "DC1394";
    configWidget = NULL;
}

ImageSourceDC1394::~ImageSourceDC1394 ()
{
}

