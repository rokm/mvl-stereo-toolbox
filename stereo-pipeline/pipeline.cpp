/*
 * Stereo Pipeline: pipeline
 * Copyright (C) 2013-2015 Rok Mandeljc
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

#include <stereo-pipeline/pipeline.h>

#include <stereo-pipeline/image_pair_source.h>
#include <stereo-pipeline/plugin_factory.h>
#include <stereo-pipeline/rectification.h>
#include <stereo-pipeline/reprojection.h>
#include <stereo-pipeline/stereo_method.h>
#include <stereo-pipeline/utils.h>

#include <opencv2/opencv_modules.hpp>
#include <opencv2/imgproc.hpp>

#ifdef HAVE_OPENCV_CUDASTEREO
#include <opencv2/cudastereo.hpp>
#endif


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


Pipeline::Pipeline (QObject *parent)
    : QObject(parent)
{
    imagePairSource = NULL;
    rectification = NULL;
    stereoMethod = NULL;
    reprojection = NULL;

    useStereoMethodThread = false;
    stereoDroppedFramesCounter = 0;

    imagePairSourceActive = true;
    rectificationActive = true;
    stereoMethodActive = true;
    reprojectionActive = true;

    disparityVisualizationMethod = DisparityVisualizationNone; // By default, turn visualization off

    connect(this, SIGNAL(inputImagesChanged()), this, SLOT(rectifyImages()));
    connect(this, SIGNAL(rectifiedImagesChanged()), this, SLOT(computeDisparityImage()));
    connect(this, SIGNAL(disparityImageChanged()), this, SLOT(reprojectDisparityImage()));
    connect(this, SIGNAL(reprojectedImageChanged()), this, SIGNAL(processingCompleted()));

    connect(this, SIGNAL(imagePairSourceStateChanged(bool)), this, SLOT(beginProcessing()));
    connect(this, SIGNAL(rectificationStateChanged(bool)), this, SLOT(rectifyImages()));
    connect(this, SIGNAL(stereoMethodStateChanged(bool)), this, SLOT(computeDisparityImage()));
    connect(this, SIGNAL(reprojectionStateChanged(bool)), this, SLOT(reprojectDisparityImage()));

    connect(this, SIGNAL(disparityVisualizationMethodChanged(int)), this, SLOT(computeDisparityImageVisualization()));

    // Create rectification
    setRectification(new Rectification(this));

    // Create reprojection
    setReprojection(new Reprojection(this));

    // Create list of supported visualization methods
    supportedDisparityVisualizationMethods.append(DisparityVisualizationNone);
    supportedDisparityVisualizationMethods.append(DisparityVisualizationGrayscale);
#ifdef HAVE_OPENCV_CUDASTEREO
    try {
        if (cv::cuda::getCudaEnabledDeviceCount()) {
            supportedDisparityVisualizationMethods.append(DisparityVisualizationColorCuda);
        }
    } catch (...) {
        // Nothing to do :)
    }
#endif
    supportedDisparityVisualizationMethods.append(DisparityVisualizationColorCpu);
}

Pipeline::~Pipeline ()
{
    // Disable all ...
    setImagePairSourceState(false);
    setRectificationState(false);
    setStereoMethodState(false);
    setReprojectionState(false);

    // ... and wait for method thread to finish
    if (stereoMethodWatcher.isRunning()) {
        stereoMethodWatcher.waitForFinished();
    }
}


// *********************************************************************
// *                        GPU/CUDA management                        *
// *********************************************************************
int Pipeline::getNumberOfGpuDevices ()
{
#ifdef HAVE_OPENCV_CUDA
    try {
        return cv::cuda::getCudaEnabledDeviceCount();
    } catch (...) {
        return 0;
    }
#else
    return 0;
#endif
}


void Pipeline::setGpuDevice (int dev)
{
#ifdef HAVE_OPENCV_CUDA
    try {
        cv::cuda::setDevice(dev);
        cv::cuda::GpuMat mat(4, 4, CV_32FC1); // Create a dummy matrix to initialize GPU
    } catch (...) {
    }
#endif
}

int Pipeline::getGpuDevice () const
{
#ifdef HAVE_OPENCV_CUDA
    return cv::cuda::getDevice();
#else
    return -1;
#endif
}


// *********************************************************************
// *                         Image pair source                         *
// *********************************************************************
// Source setting
void Pipeline::setImagePairSource (ImagePairSource *newSource)
{
    // Change source
    if (imagePairSource) {
        imagePairSource->stopSource(); // Stop the source
        disconnect(dynamic_cast<QObject *>(imagePairSource), SIGNAL(imagesChanged()), this, SLOT(beginProcessing()));

        if (dynamic_cast<QObject *>(imagePairSource)->parent() == this) {
            dynamic_cast<QObject *>(imagePairSource)->deleteLater(); // Schedule for deletion
        }
    }

    imagePairSource = newSource;
    if (!dynamic_cast<QObject *>(imagePairSource)->parent()) {
        dynamic_cast<QObject *>(imagePairSource)->setParent(this);
    }

    connect(dynamic_cast<QObject *>(imagePairSource), SIGNAL(imagesChanged()), this, SLOT(beginProcessing()));

    // Process
    beginProcessing();
}

ImagePairSource *Pipeline::getImagePairSource ()
{
    return imagePairSource;
}


// Source state
void Pipeline::setImagePairSourceState (bool newState)
{
    if (newState != imagePairSourceActive) {
        if (!newState && imagePairSource) {
            imagePairSource->stopSource(); // Stop the source
        }

        imagePairSourceActive = newState;
        emit imagePairSourceStateChanged(newState);
    }
}

bool Pipeline::getImagePairSourceState () const
{
    return imagePairSourceActive;
}


// Image retrieval
const cv::Mat &Pipeline::getLeftImage () const
{
    return inputImageL;
}

const cv::Mat &Pipeline::getRightImage () const
{
    return inputImageR;
}


// Processing
void Pipeline::beginProcessing ()
{
    // Make sure image source is marked as active
    if (!imagePairSourceActive) {
        emit processingCompleted();
        return;
    }

    // Get images from source
    imagePairSource->getImages(inputImageL, inputImageR);
    emit inputImagesChanged();
}


// *********************************************************************
// *                           Rectification                           *
// *********************************************************************
// Rectification setting
void Pipeline::setRectification (Rectification *newRectification)
{
    // Change rectification
    if (rectification) {
        disconnect(rectification, SIGNAL(stateChanged(bool)), this, SLOT(rectifyImages()));
        disconnect(rectification, SIGNAL(performRectificationChanged(bool)), this, SLOT(rectifyImages()));
        disconnect(rectification, SIGNAL(stateChanged(bool)), this, SLOT(updateReprojectionMatrix()));
        disconnect(rectification, SIGNAL(roiChanged()), this, SLOT(rectifyImages()));
        if (rectification->parent() == this) {
            rectification->deleteLater(); // Schedule for deletion
        }
    }

    rectification = newRectification;
    if (!rectification->parent()) {
        rectification->setParent(this);
    }

    connect(rectification, SIGNAL(stateChanged(bool)), this, SLOT(rectifyImages()));
    connect(rectification, SIGNAL(performRectificationChanged(bool)), this, SLOT(rectifyImages()));
    connect(rectification, SIGNAL(stateChanged(bool)), this, SLOT(updateReprojectionMatrix()));
    connect(rectification, SIGNAL(roiChanged()), this, SLOT(rectifyImages()));

    // Rectify images
    rectifyImages();
}

Rectification *Pipeline::getRectification ()
{
    return rectification;
}


// Rectification state
void Pipeline::setRectificationState (bool newState)
{
    if (newState != rectificationActive) {
        rectificationActive = newState;
        emit rectificationStateChanged(newState);
    }
}

bool Pipeline::getRectificationState () const
{
    return rectificationActive;
}


// Image retrieval
const cv::Mat &Pipeline::getLeftRectifiedImage () const
{
    return rectifiedImageL;
}

const cv::Mat &Pipeline::getRightRectifiedImage () const
{
    return rectifiedImageR;
}

int Pipeline::getRectificationTime () const
{
    return rectificationTime;
}


// Processing
void Pipeline::rectifyImages ()
{
    // Make sure we have rectification object set
    if (!rectification) {
        emit error("Stereo rectification object not set!");
        return;
    }

    // Make sure we have input images set
    if (inputImageL.empty() || inputImageR.empty()) {
        return;
    }

    // Make sure input images are of same size
    if (inputImageL.cols != inputImageR.cols || inputImageL.rows != inputImageR.rows) {
        emit error("Input images do not have same dimensions!");
        return;
    }

    // Make sure rectification is marked as active
    if (!rectificationActive) {
        emit processingCompleted();
        return;
    }

    QTime timer; timer.start();
    rectification->rectifyImagePair(inputImageL, inputImageR, rectifiedImageL, rectifiedImageR);
    rectificationTime = timer.elapsed();

    emit rectifiedImagesChanged();
}


// *********************************************************************
// *                           Stereo method                           *
// *********************************************************************
// Method setting
void Pipeline::setStereoMethod (StereoMethod *newMethod)
{
    // Change method
    if (dynamic_cast<QObject *>(stereoMethod)) {
        disconnect(dynamic_cast<QObject *>(stereoMethod), SIGNAL(parameterChanged()), this, SLOT(computeDisparityImage()));

        if (dynamic_cast<QObject *>(stereoMethod)->parent() == this) {
            dynamic_cast<QObject *>(stereoMethod)->deleteLater(); // Schedule for deletion
        }
    }

    stereoMethod = newMethod;
    if (!dynamic_cast<QObject *>(stereoMethod)->parent()) {
        dynamic_cast<QObject *>(stereoMethod)->deleteLater();
    }

    connect(dynamic_cast<QObject *>(stereoMethod), SIGNAL(parameterChanged()), this, SLOT(computeDisparityImage()));

    // Compute new disparity image
    computeDisparityImage();
}

StereoMethod *Pipeline::getStereoMethod ()
{
    return stereoMethod;
}


// Method state
void Pipeline::setStereoMethodState (bool newState)
{
    if (newState != stereoMethodActive) {
        stereoMethodActive = newState;
        emit stereoMethodStateChanged(newState);
    }
}

bool Pipeline::getStereoMethodState () const
{
    return stereoMethodActive;
}


// Image retrieval
const cv::Mat &Pipeline::getDisparityImage () const
{
    return disparityImage;
}

int Pipeline::getNumberOfDisparityLevels () const
{
    return disparityLevels;
}

int Pipeline::getDisparityImageComputationTime () const
{
    return disparityImageComputationTime;
}


// Processing
void Pipeline::computeDisparityImage ()
{
    // Make sure we have rectified images ready
    if (rectifiedImageL.empty() || rectifiedImageR.empty()) {
        return;
    }

    // Make sure stereo method is marked as active
    if (!stereoMethodActive) {
        emit processingCompleted();
        return;
    }

    // Make sure we have stereo method set
    if (!stereoMethod) {
        emit error("Stereo method not set!");
        return;
    }

    if (useStereoMethodThread) {
        // Start processing if not processing already; otherwise drop
        if (!stereoMethodWatcher.isRunning()) {
            stereoDroppedFramesCounter = 0;
            QFuture<void> future = QtConcurrent::run(this, &Pipeline::computeDisparityImageInThread);
            stereoMethodWatcher.setFuture(future);
        } else {
            stereoDroppedFramesCounter++;
        }
    } else {
        // Direct call
        computeDisparityImageInThread();
    }
}


void Pipeline::computeDisparityImageInThread ()
{
    // If input images are not set, clear disparity image; otherwise,
    // compute new disparity image
    if (rectifiedImageL.empty() || rectifiedImageR.empty()) {
        disparityImage = cv::Mat();
    } else {
        try {
            QTime timer; timer.start();
            // Make sure disparity image is of correct size
            disparityImage.create(rectifiedImageL.rows, rectifiedImageL.cols, CV_8UC1);

            // Compute disparity
            stereoMethod->computeDisparityImage(rectifiedImageL, rectifiedImageR, disparityImage, disparityLevels);

            disparityImageComputationTime = timer.elapsed();
        } catch (std::exception &e) {
            disparityImage = cv::Mat(); // Clear
            qWarning() << "Stereo method error: " << e.what();
            emit error(QString("Stereo method error: %1").arg(QString::fromStdString(e.what())));
        }
    }

    emit disparityImageChanged();

    // Visualize disparity
    computeDisparityImageVisualization();
}


void Pipeline::setUseStereoMethodThread (bool enable)
{
    // No-op if already set
    if (enable == useStereoMethodThread) {
        return;
    }

    useStereoMethodThread = enable;
}

bool Pipeline::getUseStereoMethodThread () const
{
    return useStereoMethodThread;
}

int Pipeline::getStereoDroppedFrames () const
{
    return stereoDroppedFramesCounter;
}


// *********************************************************************
// *                   Stereo disparity visualization                  *
// *********************************************************************
const cv::Mat &Pipeline::getDisparityVisualizationImage () const
{
    return disparityVisualizationImage;
}

void Pipeline::setDisparityVisualizationMethod (int newMethod)
{
    if (newMethod == disparityVisualizationMethod) {
        return;
    }

    // Make sure method is supported
    if (!supportedDisparityVisualizationMethods.contains(newMethod)) {
        disparityVisualizationMethod = DisparityVisualizationNone;
        emit error(QString("Reprojection method %1 not supported!").arg(newMethod));
    } else {
        disparityVisualizationMethod = newMethod;
    }

    // Emit in any case
    emit disparityVisualizationMethodChanged(disparityVisualizationMethod);
}

int Pipeline::getDisparityVisualizationMethod () const
{
    return disparityVisualizationMethod;
}

const QList<int> &Pipeline::getSupportedDisparityVisualizationMethods () const
{
    return supportedDisparityVisualizationMethods;
}


void Pipeline::computeDisparityImageVisualization ()
{
    switch (disparityVisualizationMethod) {
        case DisparityVisualizationNone: {
            disparityVisualizationImage = cv::Mat();
            break;
        }
        case DisparityVisualizationGrayscale: {
            // Raw grayscale disparity
            disparityImage.convertTo(disparityVisualizationImage, CV_8U, 255.0/disparityLevels);
            break;
        }
#ifdef HAVE_OPENCV_CUDASTEREO
        case DisparityVisualizationColorCuda: {
            try {
                // Hue-color-coded disparity
                cv::cuda::GpuMat gpu_disp(disparityImage);
                cv::cuda::GpuMat gpu_disp_color;
                cv::Mat disp_color;

                cv::cuda::drawColorDisp(gpu_disp, gpu_disp_color, disparityLevels);
                gpu_disp_color.download(disparityVisualizationImage);
            } catch (...) {
                // The above calls can fail
                disparityVisualizationImage = cv::Mat();
            }

            break;
        }
#endif
        case DisparityVisualizationColorCpu :{
            Utils::createColorCodedDisparityCpu(disparityImage, disparityVisualizationImage, disparityLevels);
            break;
        }
    }

    emit disparityVisualizationImageChanged();
}


// *********************************************************************
// *                          3D Reprojection                          *
// *********************************************************************
void Pipeline::updateReprojectionMatrix ()
{
    if (rectification && reprojection) {
        reprojection->setReprojectionMatrix(rectification->getReprojectionMatrix());
    }
}

// Reprojection setting
void Pipeline::setReprojection (Reprojection *newReprojection)
{
    // Change reprojection
    if (reprojection) {
        disconnect(reprojection, SIGNAL(reprojectionMethodChanged(int)), this, SLOT(reprojectDisparityImage()));
        if (reprojection->parent() == this) {
            reprojection->deleteLater(); // Schedule for deletion
        }
    }

    reprojection = newReprojection;
    if (!reprojection->parent()) {
        reprojection->setParent(this);
    }

    connect(reprojection, SIGNAL(reprojectionMethodChanged(int)), this, SLOT(reprojectDisparityImage()));

    // Reproject disparity image
    reprojectDisparityImage();
}

Reprojection *Pipeline::getReprojection ()
{
    return reprojection;
}


// Reprojection state
void Pipeline::setReprojectionState (bool newState)
{
    if (newState != reprojectionActive) {
        reprojectionActive = newState;
        emit reprojectionStateChanged(newState);
    }
}

bool Pipeline::getReprojectionState () const
{
    return reprojectionActive;
}


// Image retrieval
const cv::Mat &Pipeline::getReprojectedImage () const
{
    return reprojectedImage;
}

int Pipeline::getReprojectionComputationTime () const
{
    return reprojectionComputationTime;
}


// Processing
void Pipeline::reprojectDisparityImage ()
{
    // Make sure we have reprojection object set
    if (!reprojection) {
        emit error("Stereo reprojection object not set!");
        return;
    }

    // Make sure we have disparity image
    if (disparityImage.empty()) {
        return;
    }

    // Make sure reprojection is marked as active
    if (!reprojectionActive) {
        emit processingCompleted();
        return;
    }

    // Reproject
    try {
        QTime timer; timer.start();
        const cv::Rect &roi = rectification->getRoi();
        reprojection->reprojectStereoDisparity(disparityImage, reprojectedImage, roi.x, roi.y);
        reprojectionComputationTime = timer.elapsed();
    } catch (std::exception &e) {
        qWarning() << "Failed to reproject:" << QString::fromStdString(e.what());
        reprojectedImage = cv::Mat();
    }

    emit reprojectedImageChanged();
}


} // Pipeline
} // StereoToolbox
} // MVL
