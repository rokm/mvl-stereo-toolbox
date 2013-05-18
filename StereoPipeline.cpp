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


const cv::Mat &StereoPipeline::getDepthImage () const
{
    return depthImage;
}

int StereoPipeline::getDepthImageComputationTime () const
{
    return depthImageComputationTime;
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
}


// *********************************************************************
// *                         Calibration object                        *
// *********************************************************************
void StereoPipeline::setCalibration (StereoCalibration *newCalibration)
{
    calibration = newCalibration;
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

    // Compute new depth image
    computeDepthImage();
}


// *********************************************************************
// *                             Processing                            *
// *********************************************************************
void StereoPipeline::beginProcessing ()
{
    // Get images from source
    imageSource->getImages(inputImageL, inputImageR);
    
    // Rectify input images
    rectifyImages();

    emit inputImagesChanged();

    // Compute depth image
    computeDepthImage();
}

void StereoPipeline::rectifyImages ()
{
    if (!calibration) {
        rectifiedImageL = inputImageL;
        rectifiedImageR = inputImageR;
        //emit error("Stereo calibration object not set!");
        return;
    }

    calibration->rectifyImagePair(inputImageL, inputImageR, rectifiedImageL, rectifiedImageR);
}

void StereoPipeline::computeDepthImage ()
{
    if (!method) {
        emit error("Stereo method not set!");
        return;
    }

    // No-op if images are not set
    if (rectifiedImageL.empty() || rectifiedImageR.empty()) {
        return;
    }

    try {
        QTime timer; timer.start();
        method->computeDepthImage(rectifiedImageL, rectifiedImageR, depthImage);
        depthImageComputationTime = timer.elapsed();
    
        emit depthImageChanged();
    } catch (std::exception &e) {
        qWarning() << "Error: " << e.what();
    }
}

void StereoPipeline::methodParameterChanged ()
{
    // Recompute depth image using cached (rectified) input images
    computeDepthImage();
}
