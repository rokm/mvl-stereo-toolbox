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

// Config widget
#include <QtGui>

class ConfigTabDC1394 : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabDC1394 (ImageSourceDC1394 *, QWidget * = 0);
    virtual ~ConfigTabDC1394 ();

protected slots:

protected:
    ImageSourceDC1394 *source;
};

#endif
