#ifndef IMAGE_SOURCE_DC1394_H
#define IMAGE_SOURCE_DC1394_H

#include "ImageSource.h"


class ImageSourceDC1394 : public ImageSource
{
    Q_OBJECT

public:
    ImageSourceDC1394 (QObject * = 0);
    virtual ~ImageSourceDC1394 ();
};

#endif
