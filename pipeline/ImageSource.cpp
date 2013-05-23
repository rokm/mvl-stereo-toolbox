/*
 * Stereo Pipeline: image source
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
