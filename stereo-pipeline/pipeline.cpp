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
#include "rectification_element.h"
#include "method_element.h"
#include "reprojection_element.h"
#include "visualization_element.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {



PipelinePrivate::PipelinePrivate (Pipeline *parent)
    : q_ptr(parent)
{
    Q_Q(Pipeline);

    qRegisterMetaType< cv::Mat >();

    // Name the main thread, for easier debugging
    QCoreApplication::instance()->thread()->setObjectName("MainThread");

    // New element-wrapper-based API
    source = new AsyncPipeline::SourceElement(q);
    rectification = new AsyncPipeline::RectificationElement(q);
    stereoMethod = new AsyncPipeline::MethodElement(q);
    reprojection = new AsyncPipeline::ReprojectionElement(q);
    visualization = new AsyncPipeline::VisualizationElement(q);

    // Automatically propagate reprojection matrix from rectification
    // object to reprojection object
    q->connect(rectification->getRectification(), &Rectification::calibrationChanged, q, [this] (bool valid) {
        reprojection->getReprojection()->setReprojectionMatrix(rectification->getRectification()->getReprojectionMatrix());
    });

    // Propagate errors
    q->connect(source, &AsyncPipeline::SourceElement::error, q, [this, q] (const QString message) {
        emit q->error(Pipeline::ErrorImagePairSource, message);
    });
    q->connect(rectification, &AsyncPipeline::RectificationElement::error, q, [this, q] (const QString message) {
        emit q->error(Pipeline::ErrorRectification, message);
    });
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::error, q, [this, q] (const QString message) {
        emit q->error(Pipeline::ErrorStereoMethod, message);
    });
    q->connect(reprojection, &AsyncPipeline::ReprojectionElement::error, q, [this, q] (const QString message) {
        emit q->error(Pipeline::ErrorReprojection, message);
    });
    q->connect(visualization, &AsyncPipeline::VisualizationElement::error, q, [this, q] (const QString message) {
        emit q->error(Pipeline::ErrorVisualization, message);
    });

    // Propagate state changes
    q->connect(source, &AsyncPipeline::SourceElement::stateChanged, q, &Pipeline::imagePairSourceStateChanged);
    q->connect(rectification, &AsyncPipeline::RectificationElement::stateChanged, q, &Pipeline::rectificationStateChanged);
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::stateChanged, q, &Pipeline::stereoMethodStateChanged);
    q->connect(reprojection, &AsyncPipeline::ReprojectionElement::stateChanged, q, &Pipeline::reprojectionStateChanged);
    q->connect(visualization, &AsyncPipeline::VisualizationElement::stateChanged, q, &Pipeline::visualizationStateChanged);

    // Setup processing chain
    q->connect(source, &AsyncPipeline::SourceElement::framerateLimitChanged, q, &Pipeline::imageCaptureFramerateLimitChanged);
    q->connect(source, &AsyncPipeline::SourceElement::imagesChanged, q, &Pipeline::inputImagesChanged);
    q->connect(source, &AsyncPipeline::SourceElement::imagesChanged, q, &Pipeline::rectifyImages);
    q->connect(source, &AsyncPipeline::SourceElement::frameDropped, q, &Pipeline::imageCaptureFrameDropped);
    q->connect(source, &AsyncPipeline::SourceElement::frameRateReport, q, &Pipeline::imageCaptureFramerateUpdated);

    q->connect(rectification, &AsyncPipeline::RectificationElement::imagesChanged, q, &Pipeline::rectifiedImagesChanged);
    q->connect(rectification, &AsyncPipeline::RectificationElement::imagesChanged, q, &Pipeline::computeDisparity);
    q->connect(rectification, &AsyncPipeline::RectificationElement::frameDropped, q, &Pipeline::rectificationFrameDropped);
    q->connect(rectification, &AsyncPipeline::RectificationElement::frameRateReport, q, &Pipeline::rectificationFramerateUpdated);

    q->connect(stereoMethod, &AsyncPipeline::MethodElement::disparityChanged, q, &Pipeline::disparityChanged);
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::disparityChanged, q, &Pipeline::reprojectPoints);
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::disparityChanged, q, &Pipeline::visualizeDisparity);
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::frameDropped, q, &Pipeline::stereoMethodFrameDropped);
    q->connect(rectification, &AsyncPipeline::MethodElement::frameRateReport, q, &Pipeline::stereoMethodFramerateUpdated);

    q->connect(reprojection, &AsyncPipeline::ReprojectionElement::pointsChanged, q, &Pipeline::pointsChanged);
    q->connect(reprojection, &AsyncPipeline::ReprojectionElement::frameDropped, q, &Pipeline::reprojectionFrameDropped);
    q->connect(rectification, &AsyncPipeline::ReprojectionElement::frameRateReport, q, &Pipeline::reprojectionFramerateUpdated);

    q->connect(visualization, &AsyncPipeline::VisualizationElement::imageChanged, q, &Pipeline::visualizationChanged);
    q->connect(visualization, &AsyncPipeline::VisualizationElement::frameDropped, q, &Pipeline::visualizationFrameDropped);
    q->connect(visualization, &AsyncPipeline::ReprojectionElement::frameRateReport, q, &Pipeline::visualizationFramerateUpdated);

    // Re-computation of individual steps upon relevant changes in components
    q->connect(rectification, &AsyncPipeline::RectificationElement::calibrationChanged, q, &Pipeline::rectifyImages);
    q->connect(rectification, &AsyncPipeline::RectificationElement::performRectificationChanged, q, &Pipeline::rectifyImages);
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::methodChanged, q, &Pipeline::computeDisparity);
    q->connect(visualization, &AsyncPipeline::VisualizationElement::visualizationMethodChanged, q, &Pipeline::visualizeDisparity);
}


Pipeline::Pipeline (QObject *parent)
    : QObject(parent), d_ptr(new PipelinePrivate(this))
{
}

Pipeline::~Pipeline ()
{
}


// *********************************************************************
// *                         Processing steps                          *
// *********************************************************************
void Pipeline::rectifyImages ()
{
    Q_D(Pipeline);

    cv::Mat imageL, imageR;
    d->source->getImages(imageL, imageR);
    d->rectification->rectifyImages(imageL, imageR);
}

void Pipeline::computeDisparity ()
{
    Q_D(Pipeline);

    cv::Mat imageL, imageR;
    d->rectification->getImages(imageL, imageR);
    d->stereoMethod->computeDisparity(imageL, imageR);
}

void Pipeline::reprojectPoints ()
{
    Q_D(Pipeline);

    cv::Mat disparity;
    int numLevels;
    d->stereoMethod->getDisparity(disparity, numLevels);
    d->reprojection->reprojectDisparity(disparity, numLevels);
}

void Pipeline::visualizeDisparity ()
{
    Q_D(Pipeline);

    cv::Mat disparity;
    int numLevels;
    d->stereoMethod->getDisparity(disparity, numLevels);
    d->visualization->visualizeDisparity(disparity, numLevels);
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


// Element state
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
void Pipeline::getImages (cv::Mat &imageLeft, cv::Mat &imageRight) const
{
    Q_D(const Pipeline);
    return d->source->getImages(imageLeft, imageRight);
}

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


// Framerate limit
void Pipeline::setImageCaptureFramerateLimit (double limit)
{
    Q_D(Pipeline);
    return d->source->setFramerateLimit(limit);
}

double Pipeline::getImageCaptureFramerateLimit () const
{
    Q_D(const Pipeline);
    return d->source->getFramerateLimit();
}


// *********************************************************************
// *                           Rectification                           *
// *********************************************************************
Rectification *Pipeline::getRectification ()
{
    Q_D(Pipeline);
    return d->rectification->getRectification();
}


// Element state
void Pipeline::setRectificationState (bool active)
{
    Q_D(Pipeline);
    d->rectification->setState(active);
}

bool Pipeline::getRectificationState () const
{
    Q_D(const Pipeline);
    return d->rectification->getState();
}



// Rectified image retrieval
cv::Mat Pipeline::getLeftRectifiedImage () const
{
    Q_D(const Pipeline);
    return d->rectification->getLeftImage();
}

cv::Mat Pipeline::getRightRectifiedImage () const
{
    Q_D(const Pipeline);
    return d->rectification->getRightImage();
}

void Pipeline::getRectifiedImages (cv::Mat &imageLeft, cv::Mat &imageRight) const
{
    Q_D(const Pipeline);
    return d->rectification->getImages(imageLeft, imageRight);
}

// Timings
int Pipeline::getRectificationTime () const
{
    Q_D(const Pipeline);
    return d->rectification->getLastOperationTime();
}


int Pipeline::getRectificationDroppedFrames () const
{
    Q_D(const Pipeline);
    return d->rectification->getNumberOfDroppedFrames();
}

float Pipeline::getRectificationFramerate () const
{
    Q_D(const Pipeline);
    return d->rectification->getFramesPerSecond();
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


// Element state
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


// Parameters import/export
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
cv::Mat Pipeline::getDisparity () const
{
    Q_D(const Pipeline);
    return d->stereoMethod->getDisparity();
}

void Pipeline::getDisparity (cv::Mat &disparity) const
{
    Q_D(const Pipeline);
    int numDisparityLevels;
    d->stereoMethod->getDisparity(disparity, numDisparityLevels);
}

void Pipeline::getDisparity (cv::Mat &disparity, int &numDisparityLevels) const
{
    Q_D(const Pipeline);
    d->stereoMethod->getDisparity(disparity, numDisparityLevels);
}


// Timings
int Pipeline::getStereoMethodTime () const
{
    Q_D(const Pipeline);
    return d->stereoMethod->getLastOperationTime();
}


int Pipeline::getStereoMethodDroppedFrames () const
{
    Q_D(const Pipeline);
    return d->stereoMethod->getNumberOfDroppedFrames();
}

float Pipeline::getStereoMethodFramerate () const
{
    Q_D(const Pipeline);
    return d->stereoMethod->getFramesPerSecond();
}



// *********************************************************************
// *                      Disparity visualization                      *
// *********************************************************************
DisparityVisualization *Pipeline::getVisualization ()
{
    Q_D(Pipeline);
    return d->visualization->getVisualization();
}


// Element state
void Pipeline::setVisualizationState (bool active)
{
    Q_D(Pipeline);
    d->visualization->setState(active);
}

bool Pipeline::getVisualizationState () const
{
    Q_D(const Pipeline);
    return d->visualization->getState();
}



// Visualization retrieval
cv::Mat Pipeline::getDisparityVisualization () const
{
    Q_D(const Pipeline);
    return d->visualization->getImage();
}

void Pipeline::getDisparityVisualization (cv::Mat &image) const
{
    Q_D(const Pipeline);
    d->visualization->getImage(image);
}

// Timings
int Pipeline::getVisualizationTime () const
{
    Q_D(const Pipeline);
    return d->visualization->getLastOperationTime();
}


int Pipeline::getVisualizationDroppedFrames () const
{
    Q_D(const Pipeline);
    return d->visualization->getNumberOfDroppedFrames();
}

float Pipeline::getVisualizationFramerate () const
{
    Q_D(const Pipeline);
    return d->visualization->getFramesPerSecond();
}


// *********************************************************************
// *                          3D Reprojection                          *
// *********************************************************************
Reprojection *Pipeline::getReprojection ()
{
    Q_D(Pipeline);
    return d->reprojection->getReprojection();
}


// Reprojection state
void Pipeline::setReprojectionState (bool active)
{
    Q_D(Pipeline);
    d->reprojection->setState(active);
}

bool Pipeline::getReprojectionState () const
{
    Q_D(const Pipeline);
    return d->reprojection->getState();
}


cv::Mat Pipeline::getPoints () const
{
    Q_D(const Pipeline);
    return d->reprojection->getPoints();
}

void Pipeline::getPoints (cv::Mat &points) const
{
    Q_D(const Pipeline);
    d->reprojection->getPoints(points);
}


// Timings
int Pipeline::getReprojectionTime () const
{
    Q_D(const Pipeline);
    return d->reprojection->getLastOperationTime();
}


int Pipeline::getReprojectionDroppedFrames () const
{
    Q_D(const Pipeline);
    return d->reprojection->getNumberOfDroppedFrames();
}

float Pipeline::getReprojectionFramerate () const
{
    Q_D(const Pipeline);
    return d->reprojection->getFramesPerSecond();
}


} // Pipeline
} // StereoToolbox
} // MVL
