/*
 * Stereo Pipeline: stereo method
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

#include "StereoMethod.h"


StereoMethod::StereoMethod (QObject *parent)
    : QObject(parent)
{
    // Default image dimensions
    imageWidth = 640;
    imageHeight = 480;
    imageChannels = 1;
}

StereoMethod::~StereoMethod ()
{
}


// *********************************************************************
// *                            Method name                            *
// *********************************************************************
const QString &StereoMethod::getShortName () const
{
    return shortName;
}


// *********************************************************************
// *                        Image dimensions                           *
// *********************************************************************
void StereoMethod::setImageDimensions (int width, int height, int channels)
{
    if (imageWidth != width || imageHeight != height || imageChannels != channels) {
        imageWidth = width;
        imageHeight = height;
        imageChannels = channels;

        emit imageDimensionsChanged();
    }
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
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethod::loadParameters (const QString &filename)
{
    // Open storage
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::READ);
    if (!storage.isOpened()) {
        throw QString("Cannot open file \"%1\" for reading!").arg(filename);
    }

    // Load from storage
    loadParameters(storage);
}

void StereoMethod::loadParameters (const cv::FileStorage &storage)
{
    // Validate method name
    QString storedName = QString::fromStdString(storage["MethodName"]);
    if (shortName.compare(storedName)) {
        throw QString("Invalid configuration for method \"%1\"!").arg(shortName);
    }

    // Actual parameter loading is implemented by children...
}

void StereoMethod::saveParameters (const QString &filename) const
{
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::WRITE);
    if (!storage.isOpened()) {
        throw QString("Cannot open file \"%1\" for writing!").arg(filename);
    }

    // Save to storage
    saveParameters(storage);
}

void StereoMethod::saveParameters (cv::FileStorage &storage) const
{
    // Store method name, so it can be validate upon loading
    storage << "MethodName" << shortName.toStdString();

    // Actual parameter saving is implemented by children...
}

