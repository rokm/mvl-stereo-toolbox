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

#include <QtConcurrent>

#include <opencv2/opencv_modules.hpp>
#include <opencv2/imgproc.hpp>

#ifdef HAVE_OPENCV_CUDASTEREO
#include <opencv2/cudastereo.hpp>
#endif


#include "pipeline_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


Pipeline::Pipeline (QObject *parent)
    : QObject(parent), d_ptr(new PipelinePrivate(this))
{
    connect(this, &Pipeline::inputImagesChanged, this, &Pipeline::rectifyImages);
    connect(this, &Pipeline::rectifiedImagesChanged, this, &Pipeline::computeDisparityImage);
    connect(this, &Pipeline::disparityImageChanged, this, &Pipeline::reprojectDisparityImage);
    connect(this, &Pipeline::reprojectedImageChanged, this, &Pipeline::processingCompleted);

    connect(this, &Pipeline::imagePairSourceStateChanged, this, &Pipeline::beginProcessing);
    connect(this, &Pipeline::rectificationStateChanged, this, &Pipeline::rectifyImages);
    connect(this, &Pipeline::stereoMethodStateChanged, this, &Pipeline::computeDisparityImage);
    connect(this, &Pipeline::reprojectionStateChanged, this, &Pipeline::reprojectDisparityImage);

    connect(this, &Pipeline::disparityVisualizationMethodChanged, this, &Pipeline::computeDisparityImageVisualization);

    // Create rectification
    setRectification(new Rectification(this));

    // Create reprojection
    setReprojection(new Reprojection(this));
}

Pipeline::~Pipeline ()
{
    Q_D(Pipeline);

    // Disable all ...
    setImagePairSourceState(false);
    setRectificationState(false);
    setStereoMethodState(false);
    setReprojectionState(false);

    // ... and wait for method thread to finish
    if (d->stereoMethodWatcher.isRunning()) {
        d->stereoMethodWatcher.waitForFinished();
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
    Q_D(Pipeline);

    // Change source
    if (d->imagePairSource) {
        d->imagePairSource->stopSource(); // Stop the source
        disconnect(dynamic_cast<QObject *>(d->imagePairSource), SIGNAL(imagesChanged()), this, SLOT(beginProcessing()));

        if (dynamic_cast<QObject *>(d->imagePairSource)->parent() == this) {
            dynamic_cast<QObject *>(d->imagePairSource)->deleteLater(); // Schedule for deletion
        }
    }

    d->imagePairSource = newSource;
    if (!dynamic_cast<QObject *>(d->imagePairSource)->parent()) {
        dynamic_cast<QObject *>(d->imagePairSource)->setParent(this);
    }

    // NOTE: we need to use the old syntax, because signal is defined
    // in our abstract ImagePairSource interfae
    connect(dynamic_cast<QObject *>(d->imagePairSource), SIGNAL(imagesChanged()), this, SLOT(beginProcessing()));

    // Process
    beginProcessing();
}

ImagePairSource *Pipeline::getImagePairSource ()
{
    Q_D(Pipeline);
    return d->imagePairSource;
}


// Source state
void Pipeline::setImagePairSourceState (bool newState)
{
    Q_D(Pipeline);

    if (newState != d->imagePairSourceActive) {
        if (!newState && d->imagePairSource) {
            d->imagePairSource->stopSource(); // Stop the source
        }

        d->imagePairSourceActive = newState;
        emit imagePairSourceStateChanged(newState);
    }
}

bool Pipeline::getImagePairSourceState () const
{
    Q_D(const Pipeline);
    return d->imagePairSourceActive;
}


// Image retrieval
const cv::Mat &Pipeline::getLeftImage () const
{
    Q_D(const Pipeline);
    return d->inputImageL;
}

const cv::Mat &Pipeline::getRightImage () const
{
    Q_D(const Pipeline);
    return d->inputImageR;
}


// Processing
void Pipeline::beginProcessing ()
{
    Q_D(Pipeline);

    // Make sure image source is marked as active
    if (!d->imagePairSourceActive) {
        emit processingCompleted();
        return;
    }

    // Get images from source
    d->imagePairSource->getImages(d->inputImageL, d->inputImageR);
    emit inputImagesChanged();
}


// *********************************************************************
// *                           Rectification                           *
// *********************************************************************
// Rectification setting
void Pipeline::setRectification (Rectification *newRectification)
{
    Q_D(Pipeline);

    // Change rectification
    if (d->rectification) {
        disconnect(d->rectification, &Rectification::stateChanged, this, &Pipeline::rectifyImages);
        disconnect(d->rectification, &Rectification::performRectificationChanged, this, &Pipeline::rectifyImages);
        disconnect(d->rectification, &Rectification::stateChanged, this, &Pipeline::updateReprojectionMatrix);
        disconnect(d->rectification, &Rectification::roiChanged, this, &Pipeline::rectifyImages);
        if (d->rectification->parent() == this) {
            d->rectification->deleteLater(); // Schedule for deletion
        }
    }

    d->rectification = newRectification;
    if (!d->rectification->parent()) {
        d->rectification->setParent(this);
    }

    connect(d->rectification, &Rectification::stateChanged, this, &Pipeline::rectifyImages);
    connect(d->rectification, &Rectification::performRectificationChanged, this, &Pipeline::rectifyImages);
    connect(d->rectification, &Rectification::stateChanged, this, &Pipeline::updateReprojectionMatrix);
    connect(d->rectification, &Rectification::roiChanged, this, &Pipeline::rectifyImages);

    // Rectify images
    rectifyImages();
}

Rectification *Pipeline::getRectification ()
{
    Q_D(Pipeline);
    return d->rectification;
}


// Rectification state
void Pipeline::setRectificationState (bool newState)
{
    Q_D(Pipeline);

    if (newState != d->rectificationActive) {
        d->rectificationActive = newState;
        emit rectificationStateChanged(newState);
    }
}

bool Pipeline::getRectificationState () const
{
    Q_D(const Pipeline);
    return d->rectificationActive;
}


// Image retrieval
const cv::Mat &Pipeline::getLeftRectifiedImage () const
{
    Q_D(const Pipeline);
    return d->rectifiedImageL;
}

const cv::Mat &Pipeline::getRightRectifiedImage () const
{
    Q_D(const Pipeline);
    return d->rectifiedImageR;
}

int Pipeline::getRectificationTime () const
{
    Q_D(const Pipeline);
    return d->rectificationTime;
}


// Processing
void Pipeline::rectifyImages ()
{
    Q_D(Pipeline);

    // Make sure we have rectification object set
    if (!d->rectification) {
        emit error("Stereo rectification object not set!");
        return;
    }

    // Make sure we have input images set
    if (d->inputImageL.empty() || d->inputImageR.empty()) {
        return;
    }

    // Make sure input images are of same size
    if (d->inputImageL.cols != d->inputImageR.cols || d->inputImageL.rows != d->inputImageR.rows) {
        emit error("Input images do not have same dimensions!");
        return;
    }

    // Make sure rectification is marked as active
    if (!d->rectificationActive) {
        emit processingCompleted();
        return;
    }

    QTime timer; timer.start();
    d->rectification->rectifyImagePair(d->inputImageL, d->inputImageR, d->rectifiedImageL, d->rectifiedImageR);
    d->rectificationTime = timer.elapsed();

    emit rectifiedImagesChanged();
}


// *********************************************************************
// *                           Stereo method                           *
// *********************************************************************
// Method setting
void Pipeline::setStereoMethod (StereoMethod *newMethod)
{
    Q_D(Pipeline);

    // Change method
    if (dynamic_cast<QObject *>(d->stereoMethod)) {
        disconnect(dynamic_cast<QObject *>(d->stereoMethod), SIGNAL(parameterChanged()), this, SLOT(computeDisparityImage()));

        if (dynamic_cast<QObject *>(d->stereoMethod)->parent() == this) {
            dynamic_cast<QObject *>(d->stereoMethod)->deleteLater(); // Schedule for deletion
        }
    }

    d->stereoMethod = newMethod;
    if (!dynamic_cast<QObject *>(d->stereoMethod)->parent()) {
        dynamic_cast<QObject *>(d->stereoMethod)->deleteLater();
    }

    // NOTE: we need to use the old syntax, because signal is defined
    // in our abstract ImagePairSource interfae
    connect(dynamic_cast<QObject *>(d->stereoMethod), SIGNAL(parameterChanged()), this, SLOT(computeDisparityImage()));

    // Compute new disparity image
    computeDisparityImage();
}

StereoMethod *Pipeline::getStereoMethod ()
{
    Q_D(Pipeline);
    return d->stereoMethod;
}


// Method state
void Pipeline::setStereoMethodState (bool newState)
{
    Q_D(Pipeline);

    if (newState != d->stereoMethodActive) {
        d->stereoMethodActive = newState;
        emit stereoMethodStateChanged(newState);
    }
}

bool Pipeline::getStereoMethodState () const
{
    Q_D(const Pipeline);
    return d->stereoMethodActive;
}


// Image retrieval
const cv::Mat &Pipeline::getDisparityImage () const
{
    Q_D(const Pipeline);
    return d->disparityImage;
}

int Pipeline::getNumberOfDisparityLevels () const
{
    Q_D(const Pipeline);
    return d->disparityLevels;
}

int Pipeline::getDisparityImageComputationTime () const
{
    Q_D(const Pipeline);
    return d->disparityImageComputationTime;
}


// Processing
void Pipeline::computeDisparityImage ()
{
    Q_D(Pipeline);

    // Make sure we have rectified images ready
    if (d->rectifiedImageL.empty() || d->rectifiedImageR.empty()) {
        return;
    }

    // Make sure stereo method is marked as active
    if (!d->stereoMethodActive) {
        emit processingCompleted();
        return;
    }

    // Make sure we have stereo method set
    if (!d->stereoMethod) {
        emit error("Stereo method not set!");
        return;
    }

    if (d->useStereoMethodThread) {
        // Start processing if not processing already; otherwise drop
        if (!d->stereoMethodWatcher.isRunning()) {
            d->stereoDroppedFramesCounter = 0;
            QFuture<void> future = QtConcurrent::run(this, &Pipeline::computeDisparityImageInThread);
            d->stereoMethodWatcher.setFuture(future);
        } else {
            d->stereoDroppedFramesCounter++;
        }
    } else {
        // Direct call
        computeDisparityImageInThread();
    }
}


void Pipeline::computeDisparityImageInThread ()
{
    Q_D(Pipeline);

    // If input images are not set, clear disparity image; otherwise,
    // compute new disparity image
    if (d->rectifiedImageL.empty() || d->rectifiedImageR.empty()) {
        d->disparityImage = cv::Mat();
    } else {
        try {
            QTime timer; timer.start();
            // Make sure disparity image is of correct size
            d->disparityImage.create(d->rectifiedImageL.rows, d->rectifiedImageL.cols, CV_8UC1);

            // Compute disparity
            d->stereoMethod->computeDisparityImage(d->rectifiedImageL, d->rectifiedImageR, d->disparityImage, d->disparityLevels);

            d->disparityImageComputationTime = timer.elapsed();
        } catch (std::exception &e) {
            d->disparityImage = cv::Mat(); // Clear
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
    Q_D(Pipeline);

    // No-op if already set
    if (enable == d->useStereoMethodThread) {
        return;
    }

    d->useStereoMethodThread = enable;
}

bool Pipeline::getUseStereoMethodThread () const
{
    Q_D(const Pipeline);
    return d->useStereoMethodThread;
}

int Pipeline::getStereoDroppedFrames () const
{
    Q_D(const Pipeline);
    return d->stereoDroppedFramesCounter;
}


// *********************************************************************
// *                   Stereo disparity visualization                  *
// *********************************************************************
const cv::Mat &Pipeline::getDisparityVisualizationImage () const
{
    Q_D(const Pipeline);
    return d->disparityVisualizationImage;
}

void Pipeline::setDisparityVisualizationMethod (int newMethod)
{
    Q_D(Pipeline);

    if (newMethod == d->disparityVisualizationMethod) {
        return;
    }

    // Make sure method is supported
    if (!d->supportedDisparityVisualizationMethods.contains(newMethod)) {
        d->disparityVisualizationMethod = VisualizationNone;
        emit error(QString("Reprojection method %1 not supported!").arg(newMethod));
    } else {
        d->disparityVisualizationMethod = newMethod;
    }

    // Emit in any case
    emit disparityVisualizationMethodChanged(d->disparityVisualizationMethod);
}

int Pipeline::getDisparityVisualizationMethod () const
{
    Q_D(const Pipeline);
    return d->disparityVisualizationMethod;
}

const QList<int> &Pipeline::getSupportedDisparityVisualizationMethods () const
{
    Q_D(const Pipeline);
    return d->supportedDisparityVisualizationMethods;
}


void Pipeline::computeDisparityImageVisualization ()
{
    Q_D(Pipeline);

    switch (d->disparityVisualizationMethod) {
        case VisualizationNone: {
            d->disparityVisualizationImage = cv::Mat();
            break;
        }
        case VisualizationGrayscale: {
            // Raw grayscale disparity
            d->disparityImage.convertTo(d->disparityVisualizationImage, CV_8U, 255.0/d->disparityLevels);
            break;
        }
#ifdef HAVE_OPENCV_CUDASTEREO
        case VisualizationColorCuda: {
            try {
                // Hue-color-coded disparity
                cv::cuda::GpuMat gpu_disp(d->disparityImage);
                cv::cuda::GpuMat gpu_disp_color;
                cv::Mat disp_color;

                cv::cuda::drawColorDisp(gpu_disp, gpu_disp_color, d->disparityLevels);
                gpu_disp_color.download(d->disparityVisualizationImage);
            } catch (...) {
                // The above calls can fail
                d->disparityVisualizationImage = cv::Mat();
            }

            break;
        }
#endif
        case VisualizationColorCpu :{
            Utils::createColorCodedDisparityCpu(d->disparityImage, d->disparityVisualizationImage, d->disparityLevels);
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
    Q_D(Pipeline);

    if (d->rectification && d->reprojection) {
        d->reprojection->setReprojectionMatrix(d->rectification->getReprojectionMatrix());
    }
}

// Reprojection setting
void Pipeline::setReprojection (Reprojection *newReprojection)
{
    Q_D(Pipeline);

    // Change reprojection
    if (d->reprojection) {
        disconnect(d->reprojection, &Reprojection::reprojectionMethodChanged, this, &Pipeline::reprojectDisparityImage);
        if (d->reprojection->parent() == this) {
            d->reprojection->deleteLater(); // Schedule for deletion
        }
    }

    d->reprojection = newReprojection;
    if (!d->reprojection->parent()) {
        d->reprojection->setParent(this);
    }

    connect(d->reprojection, &Reprojection::reprojectionMethodChanged, this, &Pipeline::reprojectDisparityImage);

    // Reproject disparity image
    reprojectDisparityImage();
}

Reprojection *Pipeline::getReprojection ()
{
    Q_D(Pipeline);
    return d->reprojection;
}


// Reprojection state
void Pipeline::setReprojectionState (bool newState)
{
    Q_D(Pipeline);

    if (newState != d->reprojectionActive) {
        d->reprojectionActive = newState;
        emit reprojectionStateChanged(newState);
    }
}

bool Pipeline::getReprojectionState () const
{
    Q_D(const Pipeline);
    return d->reprojectionActive;
}


// Image retrieval
const cv::Mat &Pipeline::getReprojectedImage () const
{
    Q_D(const Pipeline);
    return d->reprojectedImage;
}

int Pipeline::getReprojectionComputationTime () const
{
    Q_D(const Pipeline);
    return d->reprojectionComputationTime;
}


// Processing
void Pipeline::reprojectDisparityImage ()
{
    Q_D(Pipeline);

    // Make sure we have reprojection object set
    if (!d->reprojection) {
        emit error("Stereo reprojection object not set!");
        return;
    }

    // Make sure we have disparity image
    if (d->disparityImage.empty()) {
        return;
    }

    // Make sure reprojection is marked as active
    if (!d->reprojectionActive) {
        emit processingCompleted();
        return;
    }

    // Reproject
    try {
        QTime timer; timer.start();
        const cv::Rect &roi = d->rectification->getRoi();
        d->reprojection->reprojectStereoDisparity(d->disparityImage, d->reprojectedImage, roi.x, roi.y);
        d->reprojectionComputationTime = timer.elapsed();
    } catch (std::exception &e) {
        qWarning() << "Failed to reproject:" << QString::fromStdString(e.what());
        d->reprojectedImage = cv::Mat();
    }

    emit reprojectedImageChanged();
}


} // Pipeline
} // StereoToolbox
} // MVL
