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

#include "StereoPipeline.h"

#include "ImageSource.h"
#include "StereoCalibration.h"
#include "StereoMethod.h"


StereoPipeline::StereoPipeline (QObject *parent)
    : QObject(parent)
{
    imageSource = NULL;
    calibration = NULL;
    method = NULL;
}

StereoPipeline::~StereoPipeline ()
{
}


const cv::Mat &StereoPipeline::getDisparityImage () const
{
    return disparityImage;
}

int StereoPipeline::getDisparityImageComputationTime () const
{
    return disparityImageComputationTime;
}


const cv::Mat &StereoPipeline::getLeftImage () const
{
    return inputImageL;
}

const cv::Mat &StereoPipeline::getRightImage () const
{
    return inputImageR;
}
    
const cv::Mat &StereoPipeline::getLeftRectifiedImage () const
{
    return rectifiedImageL;
}

const cv::Mat &StereoPipeline::getRightRectifiedImage () const
{
    return rectifiedImageR;
}
    
// *********************************************************************
// *                         Image source object                       *
// *********************************************************************
void StereoPipeline::setImageSource (ImageSource *newSource)
{
    // Change source
    if (imageSource) {
        disconnect(imageSource, SIGNAL(imagesChanged()), this, SLOT(beginProcessing()));
    }

    imageSource = newSource;

    connect(imageSource, SIGNAL(imagesChanged()), this, SLOT(beginProcessing()));

    // Process
    beginProcessing();
}


// *********************************************************************
// *                         Calibration object                        *
// *********************************************************************
void StereoPipeline::setCalibration (StereoCalibration *newCalibration)
{
    // Change calibration
    if (calibration) {
        disconnect(calibration, SIGNAL(stateChanged(bool)), this, SLOT(rectifyImages()));
    }
    
    calibration = newCalibration;
    connect(calibration, SIGNAL(stateChanged(bool)), this, SLOT(rectifyImages()));

    // Rectify images
    rectifyImages();
}


// *********************************************************************
// *                        Stereo method object                       *
// *********************************************************************
void StereoPipeline::setStereoMethod (StereoMethod *newMethod)
{
    // Change method
    if (method) {
        disconnect(method, SIGNAL(parameterChanged()), this, SLOT(methodParameterChanged()));
    }
    
    method = newMethod;
    connect(method, SIGNAL(parameterChanged()), this, SLOT(methodParameterChanged()));

    // Compute new disparity image
    computeDisparityImage();
}


// *********************************************************************
// *                             Processing                            *
// *********************************************************************
void StereoPipeline::beginProcessing ()
{
    // Get images from source
    imageSource->getImages(inputImageL, inputImageR);
    emit inputImagesChanged();
    
    // Rectify input images
    rectifyImages();
    
    // Compute disparity image
    computeDisparityImage();
}

void StereoPipeline::rectifyImages ()
{
    if (!calibration) {
        emit error("Stereo calibration object not set!");
        return;
    }

    calibration->rectifyImagePair(inputImageL, inputImageR, rectifiedImageL, rectifiedImageR);
    emit rectifiedImagesChanged();
}

void StereoPipeline::computeDisparityImage ()
{
    if (!method) {
        emit error("Stereo method not set!");
        return;
    }

    // If input images are not set, clear disparity image; otherwise,
    // compute new disparity image
    if (rectifiedImageL.empty() || rectifiedImageR.empty()) {
        disparityImage = cv::Mat();
    } else {
        try {
            QTime timer; timer.start();
            method->computeDisparityImage(rectifiedImageL, rectifiedImageR, disparityImage);
            disparityImageComputationTime = timer.elapsed();
        } catch (std::exception &e) {
            disparityImage = cv::Mat(); // Clear
            qWarning() << "ERROR WHILE PROCESSING: " << e.what();
        }
    }
    
    emit disparityImageChanged();
}

void StereoPipeline::methodParameterChanged ()
{
    // Recompute disparity image using cached (rectified) input images
    computeDisparityImage();
}
