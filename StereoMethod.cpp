/*
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

#include "StereoMethod.h"

StereoMethod::StereoMethod (QObject *parent)
    : QObject(parent)
{
    configWidget = NULL;

    // Default image dimensions
    imageWidth = 640;
    imageHeight = 480;
    imageChannels = 1;
}

StereoMethod::~StereoMethod ()
{
}

// *********************************************************************
// *                        Image dimensions                           *
// *********************************************************************
void StereoMethod::setImageDimensions (int width, int height, int channels)
{
    imageWidth = width;
    imageHeight = height;
    imageChannels = channels;

    emit imageDimensionsChanged();
}

int StereoMethod::getImageWidth () const
{
    return imageWidth;
}

int StereoMethod::getImageHeight () const
{
    return imageHeight;
}

int StereoMethod::getImageChannels () const
{
    return imageChannels;
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
const QString &StereoMethod::getShortName () const
{
    return shortName;
}

QWidget *StereoMethod::getConfigWidget ()
{
    return configWidget;
}
