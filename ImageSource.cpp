#include "ImageSource.h"

ImageSource::ImageSource (QObject *parent)
    : QObject(parent)
{
}

ImageSource::~ImageSource ()
{
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
