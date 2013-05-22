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
    stereoMethod = NULL;

    useStereoMethodThread = false;
    stereoMethodThread = new QThread(this);

    imageSourceActive = true;
    calibrationActive = true;
    stereoMethodActive = true;

    connect(this, SIGNAL(inputImagesChanged()), this, SLOT(rectifyImages()));
    connect(this, SIGNAL(rectifiedImagesChanged()), this, SLOT(computeDisparityImage()));

    connect(this, SIGNAL(imageSourceStateChanged(bool)), this, SLOT(beginProcessing()));
    connect(this, SIGNAL(calibrationStateChanged(bool)), this, SLOT(rectifyImages()));
    connect(this, SIGNAL(stereoMethodStateChanged(bool)), this, SLOT(computeDisparityImage()));
}

StereoPipeline::~StereoPipeline ()
{
}


// *********************************************************************
// *                            Image source                           *
// *********************************************************************
// Source setting
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


// Source state
void StereoPipeline::setImageSourceState (bool newState)
{
    if (newState != imageSourceActive) {
        imageSourceActive = newState;
        emit imageSourceStateChanged(newState);
    }
}

bool StereoPipeline::getImageSourceState () const
{
    return imageSourceActive;
}


// Image retrieval
const cv::Mat &StereoPipeline::getLeftImage () const
{
    return inputImageL;
}

const cv::Mat &StereoPipeline::getRightImage () const
{
    return inputImageR;
}


// Processing
void StereoPipeline::beginProcessing ()
{
    // Make sure image source is marked as active
    if (!imageSourceActive) {
        return;
    }
    
    // Get images from source
    imageSource->getImages(inputImageL, inputImageR);
    emit inputImagesChanged();
}


// *********************************************************************
// *                            Calibration                            *
// *********************************************************************
// Calibration setting
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


// Calibration state
void StereoPipeline::setCalibrationState (bool newState)
{
    if (newState != calibrationActive) {
        calibrationActive = newState;
        emit calibrationStateChanged(newState);
    }
}

bool StereoPipeline::getCalibrationState () const
{
    return calibrationActive;
}


// Image retrieval
const cv::Mat &StereoPipeline::getLeftRectifiedImage () const
{
    return rectifiedImageL;
}

const cv::Mat &StereoPipeline::getRightRectifiedImage () const
{
    return rectifiedImageR;
}

int StereoPipeline::getRectificationTime () const
{
    return rectificationTime;
}


// Processing
void StereoPipeline::rectifyImages ()
{
    // Make sure calibration is marked as active
    if (!calibrationActive) {
        return;
    }
    
    // Make sure we have calibration object set
    if (!calibration) {
        emit error("Stereo calibration object not set!");
        return;
    }

    QTime timer; timer.start();
    calibration->rectifyImagePair(inputImageL, inputImageR, rectifiedImageL, rectifiedImageR);
    rectificationTime = timer.elapsed();

    emit rectifiedImagesChanged();
}


// *********************************************************************
// *                           Stereo method                           *
// *********************************************************************
// Method setting
void StereoPipeline::setStereoMethod (StereoMethod *newMethod)
{
    // Change method
    if (stereoMethod) {
        disconnect(stereoMethod, SIGNAL(parameterChanged()), this, SLOT(computeDisparityImage()));
    }
    
    stereoMethod = newMethod;
    connect(stereoMethod, SIGNAL(parameterChanged()), this, SLOT(computeDisparityImage()));

    // Compute new disparity image
    computeDisparityImage();
}


// Method state
void StereoPipeline::setStereoMethodState (bool newState)
{
    if (newState != stereoMethodActive) {
        stereoMethodActive = newState;
        emit stereoMethodStateChanged(newState);
    }
}

bool StereoPipeline::getStereoMethodState () const
{
    return stereoMethodActive;
}


// Image retrieval
const cv::Mat &StereoPipeline::getDisparityImage () const
{
    return disparityImage;
}

int StereoPipeline::getNumberOfDisparityLevels () const
{
    return disparityLevels;
}

int StereoPipeline::getDisparityImageComputationTime () const
{
    return disparityImageComputationTime;
}


// Processing
void StereoPipeline::computeDisparityImage ()
{
    // Make sure stereo method is marked as active
    if (!stereoMethodActive) {
        return;
    }
    
    // Make sure we have stereo method set
    if (!stereoMethod) {
        emit error("Stereo method not set!");
        return;
    }

    if (useStereoMethodThread) {
        // Start processing not processing already; otherwise drop
        if (!stereoMethodThread->isRunning()) {
            stereoMethodThread->start();
        } else {
            qDebug() << "Dropping frame";
        }
    } else {
        // Direct call
        computeDisparityImageInThread();
    }
}


void StereoPipeline::computeDisparityImageInThread ()
{
    // If input images are not set, clear disparity image; otherwise,
    // compute new disparity image
    if (rectifiedImageL.empty() || rectifiedImageR.empty()) {
        disparityImage = cv::Mat();
    } else {
        try {
            QTime timer; timer.start();
            stereoMethod->computeDisparityImage(rectifiedImageL, rectifiedImageR, disparityImage, disparityLevels);
            disparityImageComputationTime = timer.elapsed();
        } catch (std::exception &e) {
            disparityImage = cv::Mat(); // Clear
            qWarning() << "ERROR WHILE PROCESSING: " << e.what();
        }
    }
    
    emit disparityImageChanged();
}


void StereoPipeline::setUseStereoMethodThread (bool enable)
{
    // No-op if already set
    if (enable == useStereoMethodThread) {
        return;
    }

    useStereoMethodThread = enable;
    
    if (enable) {
        connect(stereoMethodThread, SIGNAL(started()), this, SLOT(computeDisparityImageInThread()), Qt::DirectConnection);
        connect(this, SIGNAL(disparityImageChanged()), stereoMethodThread, SLOT(quit()), Qt::DirectConnection);
    } else {
        disconnect(stereoMethodThread, SIGNAL(started()), this, SLOT(computeDisparityImageInThread()));
        disconnect(this, SIGNAL(disparityImageChanged()), stereoMethodThread, SLOT(quit()));
    }
}

bool StereoPipeline::getUseStereoMethodThread () const
{
    return useStereoMethodThread;
}
