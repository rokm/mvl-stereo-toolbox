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

#include <stereo-pipeline/disparity_visualization.h>
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

#include "source_element.h"
#include "method_element.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {








PipelinePrivate::PipelinePrivate (Pipeline *parent)
    : q_ptr(parent)
{
    Q_Q(Pipeline);

    qRegisterMetaType< cv::Mat >();

    //imagePairSource = NULL;
    rectification = NULL;
    //stereoMethod = NULL;
    visualization = NULL;
    reprojection = NULL;

    //useStereoMethodThread = false;
    //stereoDroppedFramesCounter = 0;

    //imagePairSourceActive = true;
    rectificationActive = true;
    //stereoMethodActive = true;
    visualizationActive = true;
    reprojectionActive = true;

    visualizationTime = 0;

    // New element-wrapper-based API
    source = new AsyncPipeline::SourceElement(q);
    q->connect(source, &AsyncPipeline::SourceElement::error, q, [q] (QString message) {
        emit q->error(Pipeline::ErrorImagePairSource, message);
    });

    // FIXME: bridge between old and new API
    q->connect(source, &AsyncPipeline::SourceElement::imagesChanged, q, [q, this] (cv::Mat left, cv::Mat right) {
        left.copyTo(inputImageL);
        right.copyTo(inputImageR);
        q->rectifyImages();

        emit q->inputImagesChanged(inputImageL, inputImageR);
    });

    stereoMethod = new AsyncPipeline::MethodElement(q);
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::error, q, [q] (QString message) {
        emit q->error(Pipeline::ErrorStereoMethod, message);
    });

    q->connect(q, &Pipeline::rectifiedImagesChanged, stereoMethod, [q, this] () {
        stereoMethod->computeDisparity(rectifiedImageL, rectifiedImageR);
    });

    // FIXME: bridge between old and new API
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::disparityChanged, q, [q, this] (cv::Mat disparity, int numDisparityLevels) {
        disparity.copyTo(this->disparity);
        this->disparityLevels = numDisparityLevels;

        q->computeDisparityVisualization(); // :(
        emit q->disparityChanged();
    });
}


Pipeline::Pipeline (QObject *parent)
    : QObject(parent), d_ptr(new PipelinePrivate(this))
{
    //connect(this, &Pipeline::inputImagesChanged, this, &Pipeline::rectifyImages);
    //connect(this, &Pipeline::rectifiedImagesChanged, this, &Pipeline::computeDisparity);
    connect(this, &Pipeline::disparityChanged, this, &Pipeline::reprojectPoints);
    connect(this, &Pipeline::pointCloudChanged, this, &Pipeline::processingCompleted);

    connect(this, &Pipeline::imagePairSourceStateChanged, this, &Pipeline::beginProcessing);
    connect(this, &Pipeline::rectificationStateChanged, this, &Pipeline::rectifyImages);
    //connect(this, &Pipeline::stereoMethodStateChanged, this, &Pipeline::computeDisparity);
    connect(this, &Pipeline::reprojectionStateChanged, this, &Pipeline::reprojectPoints);

    // Create rectification
    setRectification(new Rectification(this));

    // Create visualization
    setVisualization(new DisparityVisualization(this));

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
    /*if (d->stereoMethodWatcher.isRunning()) {
        d->stereoMethodWatcher.waitForFinished();
    }*/
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
void Pipeline::setImagePairSource (QObject *source)
{
    Q_D(Pipeline);
    d->source->setImagePairSource(source);
}

QObject *Pipeline::getImagePairSource ()
{
    Q_D(Pipeline);
    return d->source->getImagePairSource();
}


// Source state
void Pipeline::setImagePairSourceState (bool active)
{
    Q_D(Pipeline);
    d->source->setState(active);
}

bool Pipeline::getImagePairSourceState () const
{
    Q_D(const Pipeline);
    return d->source->getState();
}


// Image retrieval
cv::Mat Pipeline::getLeftImage () const
{
    Q_D(const Pipeline);
    return d->source->getLeftImage();
}

cv::Mat Pipeline::getRightImage () const
{
    Q_D(const Pipeline);
    return d->source->getRightImage();
}


// Processing
void Pipeline::beginProcessing ()
{
    Q_D(Pipeline);

    // Make sure image source is marked as active
    if (!d->source->getState()) {
        emit processingCompleted();
        return;
    }

    // Get images from source
    d->source->getImages(d->inputImageL, d->inputImageR);
    emit inputImagesChanged(d->inputImageL, d->inputImageR);
}


#if 0
// Source setting
void Pipeline::setImagePairSource (ImagePairSource *source)
{
    Q_D(Pipeline);

    // Change source
    if (d->imagePairSource) {
        d->imagePairSource->stopSource(); // Stop the source
        disconnect(dynamic_cast<QObject *>(d->imagePairSource), SIGNAL(imagesChanged()), this, SLOT(beginProcessing()));
        disconnect(dynamic_cast<QObject *>(d->imagePairSource), SIGNAL(error(const QString &)), this, SLOT(propagateImagePairSourceError(const QString &)));

        if (dynamic_cast<QObject *>(d->imagePairSource)->parent() == this) {
            dynamic_cast<QObject *>(d->imagePairSource)->deleteLater(); // Schedule for deletion
        }
    }

    d->imagePairSource = source;
    if (!dynamic_cast<QObject *>(d->imagePairSource)->parent()) {
        dynamic_cast<QObject *>(d->imagePairSource)->setParent(this);
    }

    // NOTE: we need to use the old syntax, because signal is defined
    // in our abstract ImagePairSource interfae
    connect(dynamic_cast<QObject *>(d->imagePairSource), SIGNAL(imagesChanged()), this, SLOT(beginProcessing()));
    connect(dynamic_cast<QObject *>(d->imagePairSource), SIGNAL(error(const QString &)), this, SLOT(propagateImagePairSourceError(const QString &)));

    // Process
    beginProcessing();
}

ImagePairSource *Pipeline::getImagePairSource ()
{
    Q_D(Pipeline);
    return d->imagePairSource;
}


void Pipeline::propagateImagePairSourceError (const QString &errorMessage)
{
    // Propagate the error message with image pair source domain
    emit error(ErrorImagePairSource, errorMessage);
}


// Source state
void Pipeline::setImagePairSourceState (bool active)
{
    Q_D(Pipeline);

    if (active != d->imagePairSourceActive) {
        if (!active && d->imagePairSource) {
            d->imagePairSource->stopSource(); // Stop the source
        }

        d->imagePairSourceActive = active;
        emit imagePairSourceStateChanged(active);
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
    if (!d->source->getState()) {
        emit processingCompleted();
        return;
    }

    // Get images from source
    d->imagePairSource->getImages(d->inputImageL, d->inputImageR);
    emit inputImagesChanged();
}
#endif



// *********************************************************************
// *                           Rectification                           *
// *********************************************************************
// Rectification setting
void Pipeline::setRectification (Rectification *rectification)
{
    Q_D(Pipeline);

    // Change rectification
    if (d->rectification) {
        disconnect(d->rectification, &Rectification::stateChanged, this, &Pipeline::rectifyImages);
        disconnect(d->rectification, &Rectification::performRectificationChanged, this, &Pipeline::rectifyImages);
        disconnect(d->rectification, &Rectification::stateChanged, this, &Pipeline::updateReprojectionMatrix);
        if (d->rectification->parent() == this) {
            d->rectification->deleteLater(); // Schedule for deletion
        }
    }

    d->rectification = rectification;
    if (!d->rectification->parent()) {
        d->rectification->setParent(this);
    }

    connect(d->rectification, &Rectification::stateChanged, this, &Pipeline::rectifyImages);
    connect(d->rectification, &Rectification::performRectificationChanged, this, &Pipeline::rectifyImages);
    connect(d->rectification, &Rectification::stateChanged, this, &Pipeline::updateReprojectionMatrix);

    // Rectify images
    rectifyImages();
}

Rectification *Pipeline::getRectification ()
{
    Q_D(Pipeline);
    return d->rectification;
}


// Rectification state
void Pipeline::setRectificationState (bool active)
{
    Q_D(Pipeline);

    if (active != d->rectificationActive) {
        d->rectificationActive = active;
        emit rectificationStateChanged(active);
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
        emit error(ErrorGeneral, "Stereo rectification object not set!");
        return;
    }

    // Make sure we have input images set
    if (d->inputImageL.empty() || d->inputImageR.empty()) {
        return;
    }

    // Make sure input images are of same size
    if (d->inputImageL.cols != d->inputImageR.cols || d->inputImageL.rows != d->inputImageR.rows) {
        emit error(ErrorRectification, "Input images do not have same dimensions!");
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
void Pipeline::setStereoMethod (QObject *method)
{
    Q_D(Pipeline);
    d->stereoMethod->setStereoMethod(method);
}

QObject *Pipeline::getStereoMethod ()
{
    Q_D(Pipeline);
    return d->stereoMethod->getStereoMethod();
}


// Method state
void Pipeline::setStereoMethodState (bool active)
{
    Q_D(Pipeline);
    d->stereoMethod->setState(active);
}

bool Pipeline::getStereoMethodState () const
{
    Q_D(const Pipeline);
    return d->stereoMethod->getState();
}



void Pipeline::loadStereoMethodParameters (const QString &filename)
{
    Q_D(Pipeline);
    d->stereoMethod->loadParameters(filename);
}

void Pipeline::saveStereoMethodParameters (const QString &filename)
{
    Q_D(Pipeline);
    d->stereoMethod->saveParameters(filename);
}


// Disparity retrieval
const cv::Mat &Pipeline::getDisparity () const
{
    Q_D(const Pipeline);
    return d->disparity;
}


// FIXME
int Pipeline::getNumberOfDisparityLevels () const
{
    Q_D(const Pipeline);
    return d->disparityLevels;
}

int Pipeline::getDisparityComputationTime () const
{
    Q_D(const Pipeline);
    return d->stereoMethod->getLastOperationTime();
}


int Pipeline::getStereoDroppedFrames () const
{
    Q_D(const Pipeline);
    return d->stereoMethod->getNumberOfDroppedFrames();
}


#if 0
// Method setting
void Pipeline::setStereoMethod (StereoMethod *method)
{
    Q_D(Pipeline);

    // Change method
    if (dynamic_cast<QObject *>(d->stereoMethod)) {
        disconnect(dynamic_cast<QObject *>(d->stereoMethod), SIGNAL(parameterChanged()), this, SLOT(computeDisparity()));

        if (dynamic_cast<QObject *>(d->stereoMethod)->parent() == this) {
            dynamic_cast<QObject *>(d->stereoMethod)->deleteLater(); // Schedule for deletion
        }
    }

    d->stereoMethod = method;
    if (!dynamic_cast<QObject *>(d->stereoMethod)->parent()) {
        dynamic_cast<QObject *>(d->stereoMethod)->deleteLater();
    }

    // NOTE: we need to use the old syntax, because signal is defined
    // in our abstract ImagePairSource interfae
    connect(dynamic_cast<QObject *>(d->stereoMethod), SIGNAL(parameterChanged()), this, SLOT(computeDisparity()));

    // Compute new disparity image
    computeDisparity();
}

StereoMethod *Pipeline::getStereoMethod ()
{
    Q_D(Pipeline);
    return d->stereoMethod;
}


// Method state
void Pipeline::setStereoMethodState (bool active)
{
    Q_D(Pipeline);

    if (active != d->stereoMethodActive) {
        d->stereoMethodActive = active;
        emit stereoMethodStateChanged(active);
    }
}

bool Pipeline::getStereoMethodState () const
{
    Q_D(const Pipeline);
    return d->stereoMethodActive;
}


// Image retrieval
const cv::Mat &Pipeline::getDisparity () const
{
    Q_D(const Pipeline);
    return d->disparity;
}

int Pipeline::getNumberOfDisparityLevels () const
{
    Q_D(const Pipeline);
    return d->disparityLevels;
}

int Pipeline::getDisparityComputationTime () const
{
    Q_D(const Pipeline);
    return d->disparityTime;
}


// Processing
void Pipeline::computeDisparity ()
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
        emit error(ErrorGeneral, "Stereo method not set!");
        return;
    }

    if (d->useStereoMethodThread) {
        // Start processing if not processing already; otherwise drop
        if (!d->stereoMethodWatcher.isRunning()) {
            d->stereoDroppedFramesCounter = 0;
            QFuture<void> future = QtConcurrent::run(this, &Pipeline::computeDisparityInThread);
            d->stereoMethodWatcher.setFuture(future);
        } else {
            d->stereoDroppedFramesCounter++;
        }
    } else {
        // Direct call
        computeDisparityInThread();
    }
}


void Pipeline::computeDisparityInThread ()
{
    Q_D(Pipeline);

    // If input images are not set, clear disparity image; otherwise,
    // compute new disparity image
    if (d->rectifiedImageL.empty() || d->rectifiedImageR.empty()) {
        d->disparity = cv::Mat();
    } else {
        try {
            // Make a copy - FIXME: add a lock?
            cv::Mat imageL, imageR;
            d->rectifiedImageL.copyTo(imageL);
            d->rectifiedImageR.copyTo(imageR);

            QTime timer; timer.start();

            // Make sure disparity image is of correct size
            d->disparity.create(imageL.rows, imageL.cols, CV_32FC1);

            // Compute disparity
            d->stereoMethod->computeDisparity(imageL, imageR, d->disparity, d->disparityLevels);

            // Store left image for point-cloud RGB data
            imageL.copyTo(d->pointCloudRgb);

            d->disparityTime = timer.elapsed();
        } catch (std::exception &e) {
            d->disparity = cv::Mat(); // Clear
            emit error(ErrorStereoMethod, QString::fromStdString(e.what()));
        }
    }

    emit disparityChanged();

    // Visualize disparity
    computeDisparityVisualization();
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
#endif



// *********************************************************************
// *                      Disparity visualization                      *
// *********************************************************************
// Visualization setting
void Pipeline::setVisualization (DisparityVisualization *visualization)
{
    Q_D(Pipeline);

    // Change visualization
    if (d->visualization) {
        disconnect(d->visualization, &DisparityVisualization::visualizationMethodChanged, this, &Pipeline::computeDisparityVisualization);
        if (d->visualization->parent() == this) {
            d->visualization->deleteLater(); // Schedule for deletion
        }
    }

    d->visualization = visualization;
    if (!d->visualization->parent()) {
        d->visualization->setParent(this);
    }

    connect(d->visualization, &DisparityVisualization::visualizationMethodChanged, this, &Pipeline::computeDisparityVisualization);

    // Compute visualization
    computeDisparityVisualization();
}

DisparityVisualization *Pipeline::getVisualization ()
{
    Q_D(Pipeline);
    return d->visualization;
}


// Reprojection state
void Pipeline::setVisualizationState (bool active)
{
    Q_D(Pipeline);

    if (active != d->visualizationActive) {
        d->visualizationActive = active;
        emit visualizationStateChanged(active);
    }
}

bool Pipeline::getVisualizationState () const
{
    Q_D(const Pipeline);
    return d->visualizationActive;
}


// Visualization retrieval
const cv::Mat &Pipeline::getDisparityVisualization () const
{
    Q_D(const Pipeline);
    return d->disparityVisualization;
}

int Pipeline::getVisualizationTime () const
{
    Q_D(const Pipeline);
    return d->visualizationTime;
}

// Processing
void Pipeline::computeDisparityVisualization ()
{
    Q_D(Pipeline);

    // Make sure we have visualization object set
    if (!d->visualization) {
        emit error(ErrorGeneral, "Disparity visualization object not set!");
        return;
    }

    // Make sure we have disparity image
    if (d->disparity.empty()) {
        return;
    }

    // Make sure reprojection is marked as active
    if (!d->visualizationActive) {
        return;
    }

    // Visualize
    try {
        QTime timer; timer.start();
        d->visualization->visualizeDisparity(d->disparity, d->disparityLevels, d->disparityVisualization);
        d->visualizationTime = timer.elapsed();
    } catch (std::exception &e) {
        emit error(ErrorVisualization, QString::fromStdString(e.what()));
        d->disparityVisualization = cv::Mat();
    }

    emit disparityVisualizationChanged();
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
void Pipeline::setReprojection (Reprojection *reprojection)
{
    Q_D(Pipeline);

    // Change reprojection
    if (d->reprojection) {
        disconnect(d->reprojection, &Reprojection::reprojectionMethodChanged, this, &Pipeline::reprojectPoints);
        if (d->reprojection->parent() == this) {
            d->reprojection->deleteLater(); // Schedule for deletion
        }
    }

    d->reprojection = reprojection;
    if (!d->reprojection->parent()) {
        d->reprojection->setParent(this);
    }

    connect(d->reprojection, &Reprojection::reprojectionMethodChanged, this, &Pipeline::reprojectPoints);

    // Reproject points
    reprojectPoints();
}

Reprojection *Pipeline::getReprojection ()
{
    Q_D(Pipeline);
    return d->reprojection;
}


// Reprojection state
void Pipeline::setReprojectionState (bool active)
{
    Q_D(Pipeline);

    if (active != d->reprojectionActive) {
        d->reprojectionActive = active;
        emit reprojectionStateChanged(active);
    }
}

bool Pipeline::getReprojectionState () const
{
    Q_D(const Pipeline);
    return d->reprojectionActive;
}


// Point-cloud retrieval
void Pipeline::getPointCloud (cv::Mat &xyz, cv::Mat &rgb) const
{
    Q_D(const Pipeline);
    d->pointCloudXyz.copyTo(xyz);
    d->pointCloudRgb.copyTo(rgb);
}

const cv::Mat &Pipeline::getPointCloudXyz () const
{
    Q_D(const Pipeline);
    return d->pointCloudXyz;
}

const cv::Mat &Pipeline::getPointCloudRgb () const
{
    Q_D(const Pipeline);
    return d->pointCloudRgb;
}

int Pipeline::getReprojectionTime () const
{
    Q_D(const Pipeline);
    return d->reprojectionTime;
}


// Processing
void Pipeline::reprojectPoints ()
{
    Q_D(Pipeline);

    // Make sure we have reprojection object set
    if (!d->reprojection) {
        emit error(ErrorGeneral, "Stereo reprojection object not set!");
        return;
    }

    // Make sure we have disparity image
    if (d->disparity.empty()) {
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
        d->reprojection->reprojectStereoDisparity(d->disparity, d->pointCloudXyz);
        d->reprojectionTime = timer.elapsed();
    } catch (std::exception &e) {
        emit error(ErrorReprojection, QString::fromStdString(e.what()));
        d->pointCloudXyz = cv::Mat();
    }

    emit pointCloudChanged();
}


} // Pipeline
} // StereoToolbox
} // MVL
