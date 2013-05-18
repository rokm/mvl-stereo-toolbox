#ifndef IMAGE_SOURCE_FILE_H
#define IMAGE_SOURCE_FILE_H

#include "ImageSource.h"


class ImageSourceFile : public ImageSource
{
    Q_OBJECT

public:
    ImageSourceFile (QObject * = 0);
    virtual ~ImageSourceFile ();
};

#endif
