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

#include "StereoCalibration.h"
#include "StereoMethod.h"


StereoPipeline::StereoPipeline (QObject *parent)
    : QObject(parent)
{
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
    if (method) {
        disconnect(method, SIGNAL(parameterChanged()), this, SLOT(methodParameterChanged()));
    }
    
    method = newMethod;
    connect(method, SIGNAL(parameterChanged()), this, SLOT(methodParameterChanged()));
}


// *********************************************************************
// *                             Processing                            *
// *********************************************************************
void StereoPipeline::processImagePair (const cv::Mat &left, const cv::Mat &right)
{
    // Store images
    inputImageL = left;
    inputImageR = right;

    // Rectify input images
    rectifyImages();

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

    // If we do not have rectified images, do nothing
    if (rectifiedImageL.empty() || rectifiedImageR.empty()) {
        emit error("Images not set!");
        return;
    }

    method->computeDepthImage(rectifiedImageL, rectifiedImageR, depthImage);
    
    emit depthImageChanged();
}

void StereoPipeline::methodParameterChanged ()
{
    // Recompute depth image using cached (rectified) input images
    computeDepthImage();
}
