/*
 * Stereo Pipeline: image pair source
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "image_pair_source.h"


ImagePairSource::ImagePairSource ()
{
}

ImagePairSource::~ImagePairSource ()
{
}

// *********************************************************************
// *                            Source name                            *
// *********************************************************************
const QString &ImagePairSource::getShortName () const
{
    return shortName;
}


// *********************************************************************
// *                          Image retrieval                          *
// *********************************************************************
void ImagePairSource::getImages (cv::Mat &left, cv::Mat &right)
{
    // Copy images under lock, in case source implements dynamic image grab
    QReadLocker lock(&imagesLock);
    
    left = imageLeft;
    right = imageRight;
}


// *********************************************************************
// *                            Source stop                            *
// *********************************************************************
void ImagePairSource::stopSource ()
{
    // Default implementation does nothing
}
