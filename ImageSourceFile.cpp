#include "ImageSourceFile.h"

ImageSourceFile::ImageSourceFile (QObject *parent)
    : ImageSource(parent)
{
    shortName = "FILE";
    configWidget = NULL;
}

ImageSourceFile::~ImageSourceFile ()
{
}

