/*
 * Stereo Pipeline: stereo method
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

#include "stereo_method.h"


StereoMethod::StereoMethod ()
{
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
    // Validate data type
    QString dataType = QString::fromStdString(storage["DataType"]);
    if (dataType.compare("StereoMethodParameters")) {
        throw QString("Invalid stereo method parameters configuration!");
    }
    
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
    // Data type
    storage << "DataType" << "StereoMethodParameters";
    
    // Store method name, so it can be validate upon loading
    storage << "MethodName" << shortName.toStdString();

    // Actual parameter saving is implemented by children...
}

