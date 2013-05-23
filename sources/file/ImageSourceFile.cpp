/*
 * File Image Source: source
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include "ImageSourceFile.h"
#include "ImageSourceFileConfigWidget.h"

#include <opencv2/highgui/highgui.hpp>


ImageSourceFile::ImageSourceFile (QObject *parent)
    : ImageSource(parent)
{
    shortName = "FILE";
    configWidget = new ImageSourceFileConfigWidget(this);
}

ImageSourceFile::~ImageSourceFile ()
{
    // Unparent the config widget and destroy it
    configWidget->setParent(0);
    delete configWidget;
}


void ImageSourceFile::loadImagePair (const QString &left, const QString &right)
{
    filenameLeft = left;
    imageLeft = cv::imread(filenameLeft.toStdString());
    
    filenameRight = right;
    imageRight = cv::imread(filenameRight.toStdString());

    emit imagesChanged();
}


// *********************************************************************
// *                          Left image info                          *
// *********************************************************************
const QString &ImageSourceFile::getLeftFilename () const
{
    return filenameLeft;
}

int ImageSourceFile::getLeftWidth () const
{
    return imageLeft.cols;
}

int ImageSourceFile::getLeftHeight () const
{
    return imageLeft.rows;
}

int ImageSourceFile::getLeftChannels () const
{
    return imageLeft.channels();
}


// *********************************************************************
// *                         Right image info                          *
// *********************************************************************
const QString &ImageSourceFile::getRightFilename () const
{
    return filenameRight;
}

int ImageSourceFile::getRightWidth () const
{
    return imageRight.cols;
}

int ImageSourceFile::getRightHeight () const
{
    return imageRight.rows;
}

int ImageSourceFile::getRightChannels () const
{
    return imageRight.channels();
}
