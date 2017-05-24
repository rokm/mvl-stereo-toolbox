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

    // Propagate errors; also, turn off the offending element
    q->connect(source, &AsyncPipeline::SourceElement::error, q, [this, q] (const QString message) {
        source->setState(false);
        emit q->error(Pipeline::ErrorImagePairSource, message);
    });
    q->connect(rectification, &AsyncPipeline::MethodElement::error, q, [this, q] (const QString message) {
        rectification->setState(false);
        emit q->error(Pipeline::ErrorRectification, message);
    });
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::error, q, [this, q] (const QString message) {
        stereoMethod->setState(false);
        emit q->error(Pipeline::ErrorStereoMethod, message);
    });
    q->connect(reprojection, &AsyncPipeline::ReprojectionElement::error, q, [this, q] (const QString message) {
        reprojection->setState(false);
        emit q->error(Pipeline::ErrorReprojection, message);
    });
    q->connect(visualization, &AsyncPipeline::VisualizationElement::error, q, [this, q] (const QString message) {
        visualization->setState(false);
        emit q->error(Pipeline::ErrorVisualization, message);
    });

    // Setup processing chain
    q->connect(source, &AsyncPipeline::SourceElement::imagesChanged, q, &Pipeline::inputImagesChanged);
    q->connect(source, &AsyncPipeline::SourceElement::imagesChanged, rectification, &AsyncPipeline::RectificationElement::rectifyImages);

    q->connect(rectification, &AsyncPipeline::RectificationElement::imagesChanged, q, &Pipeline::rectifiedImagesChanged);
    q->connect(rectification, &AsyncPipeline::RectificationElement::imagesChanged, stereoMethod, &AsyncPipeline::MethodElement::computeDisparity);

    q->connect(stereoMethod, &AsyncPipeline::MethodElement::disparityChanged, q, &Pipeline::disparityChanged);
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::disparityChanged, reprojection, &AsyncPipeline::ReprojectionElement::reprojectDisparity);
    q->connect(stereoMethod, &AsyncPipeline::MethodElement::disparityChanged, visualization, &AsyncPipeline::VisualizationElement::visualizeDisparity);

    q->connect(reprojection, &AsyncPipeline::ReprojectionElement::pointsChanged, q, &Pipeline::pointsChanged);
    q->connect(visualization, &AsyncPipeline::VisualizationElement::imageChanged, q, &Pipeline::visualizationChanged);
}


Pipeline::Pipeline (QObject *parent)
    : QObject(parent), d_ptr(new PipelinePrivate(this))
{
}

Pipeline::~Pipeline ()
{
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


} // Pipeline
} // StereoToolbox
} // MVL
