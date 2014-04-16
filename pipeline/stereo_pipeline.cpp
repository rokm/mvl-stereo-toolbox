/*
 * Stereo Pipeline: pipeline
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

#include "stereo_pipeline.h"

#include "image_pair_source.h"
#include "plugin_factory.h"
#include "stereo_rectification.h"
#include "stereo_method.h"
#include "stereo_reprojection.h"

#include <opencv2/imgproc/imgproc.hpp>

#ifdef HAVE_OPENCV_GPU
#include <opencv2/gpu/gpu.hpp>
#endif


StereoPipeline::StereoPipeline (QObject *parent)
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
    setRectification(new StereoRectification(this));

    // Create reprojection
    setReprojection(new StereoReprojection(this));

    // Create list of supported visualization methods
    supportedDisparityVisualizationMethods.append(DisparityVisualizationNone);
    supportedDisparityVisualizationMethods.append(DisparityVisualizationGrayscale);
#ifdef HAVE_OPENCV_GPU
    try {
        if (cv::gpu::getCudaEnabledDeviceCount()) {
            supportedDisparityVisualizationMethods.append(DisparityVisualizationColorGpu);
        }
    } catch (...) {
        // Nothing to do :)
    }
#endif
}

StereoPipeline::~StereoPipeline ()
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
int StereoPipeline::getNumberOfGpuDevices ()
{
#ifdef HAVE_OPENCV_GPU
    try {
        return cv::gpu::getCudaEnabledDeviceCount();
    } catch (...) {
        return 0;
    }
#else
    return 0;
#endif
}


void StereoPipeline::setGpuDevice (int dev)
{
#ifdef HAVE_OPENCV_GPU
    try {
        cv::gpu::setDevice(dev);
        cv::gpu::GpuMat mat(4, 4, CV_32FC1); // Create a dummy matrix to initialize GPU
    } catch (...) {
    }
#endif
}

int StereoPipeline::getGpuDevice () const
{
#ifdef HAVE_OPENCV_GPU
    return cv::gpu::getDevice();
#else
    return -1;
#endif
}


// *********************************************************************
// *                         Image pair source                         *
// *********************************************************************
// Source setting
void StereoPipeline::setImagePairSource (ImagePairSource *newSource)
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

ImagePairSource *StereoPipeline::getImagePairSource ()
{
    return imagePairSource;
}


// Source state
void StereoPipeline::setImagePairSourceState (bool newState)
{
    if (newState != imagePairSourceActive) {
        if (!newState && imagePairSource) {
            imagePairSource->stopSource(); // Stop the source
        }

        imagePairSourceActive = newState;
        emit imagePairSourceStateChanged(newState);
    }
}

bool StereoPipeline::getImagePairSourceState () const
{
    return imagePairSourceActive;
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
void StereoPipeline::setRectification (StereoRectification *newRectification)
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

StereoRectification *StereoPipeline::getRectification ()
{
    return rectification;
}


// Rectification state
void StereoPipeline::setRectificationState (bool newState)
{
    if (newState != rectificationActive) {
        rectificationActive = newState;
        emit rectificationStateChanged(newState);
    }
}

bool StereoPipeline::getRectificationState () const
{
    return rectificationActive;
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
void StereoPipeline::setStereoMethod (StereoMethod *newMethod)
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

StereoMethod *StereoPipeline::getStereoMethod ()
{
    return stereoMethod;
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
            QFuture<void> future = QtConcurrent::run(this, &StereoPipeline::computeDisparityImageInThread);
            stereoMethodWatcher.setFuture(future);
        } else {
            stereoDroppedFramesCounter++;
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


void StereoPipeline::setUseStereoMethodThread (bool enable)
{
    // No-op if already set
    if (enable == useStereoMethodThread) {
        return;
    }

    useStereoMethodThread = enable;
}

bool StereoPipeline::getUseStereoMethodThread () const
{
    return useStereoMethodThread;
}

int StereoPipeline::getStereoDroppedFrames () const
{
    return stereoDroppedFramesCounter;
}


// *********************************************************************
// *                   Stereo disparity visualization                  *
// *********************************************************************
const cv::Mat &StereoPipeline::getDisparityVisualizationImage () const
{
    return disparityVisualizationImage;
}

void StereoPipeline::setDisparityVisualizationMethod (int newMethod)
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

int StereoPipeline::getDisparityVisualizationMethod () const
{
    return disparityVisualizationMethod;
}

const QList<int> &StereoPipeline::getSupportedDisparityVisualizationMethods () const
{
    return supportedDisparityVisualizationMethods;
}


void StereoPipeline::computeDisparityImageVisualization ()
{
    switch (disparityVisualizationMethod) {
        case DisparityVisualizationGrayscale: {
            // Raw grayscale disparity
            disparityImage.convertTo(disparityVisualizationImage, CV_8U, 255.0/disparityLevels);
            break;
        }
#ifdef HAVE_OPENCV_GPU
        case DisparityVisualizationColorGpu: {
            try {
                // Hue-color-coded disparity
                cv::gpu::GpuMat gpu_disp(disparityImage);
                cv::gpu::GpuMat gpu_disp_color;
                cv::Mat disp_color;

                cv::gpu::drawColorDisp(gpu_disp, gpu_disp_color, disparityLevels);
                gpu_disp_color.download(disparityVisualizationImage);
            } catch (...) {
                // The above calls can fail
                disparityVisualizationImage = cv::Mat();
            }

            break;
        }
#endif
    }

    emit disparityVisualizationImageChanged();
}


// *********************************************************************
// *                          3D Reprojection                          *
// *********************************************************************
void StereoPipeline::updateReprojectionMatrix ()
{
    if (rectification && reprojection) {
        reprojection->setReprojectionMatrix(rectification->getReprojectionMatrix());
    }
}

// Reprojection setting
void StereoPipeline::setReprojection (StereoReprojection *newReprojection)
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

StereoReprojection *StereoPipeline::getReprojection ()
{
    return reprojection;
}


// Reprojection state
void StereoPipeline::setReprojectionState (bool newState)
{
    if (newState != reprojectionActive) {
        reprojectionActive = newState;
        emit reprojectionStateChanged(newState);
    }
}

bool StereoPipeline::getReprojectionState () const
{
    return reprojectionActive;
}


// Image retrieval
const cv::Mat &StereoPipeline::getReprojectedImage () const
{
    return reprojectedImage;
}

int StereoPipeline::getReprojectionComputationTime () const
{
    return reprojectionComputationTime;
}


// Processing
void StereoPipeline::reprojectDisparityImage ()
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


// *********************************************************************
// *                            Data export                            *
// *********************************************************************
void StereoPipeline::writeMatrixToBinaryFile (const cv::Mat &matrix, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        throw QString("Failed to open file!");
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Signature - binary matrix dump
    stream << (quint8)'B';
    stream << (quint8)'M';
    stream << (quint8)'D';
    stream << (quint8)' ';

    stream << (quint32)matrix.cols; // Width
    stream << (quint32)matrix.rows; // Height
    stream << (quint16)matrix.channels(); // Number of channels
    stream << (quint16)matrix.depth(); // Depth

    for (int y = 0; y < matrix.rows; y++) {
        for (int x = 0; x < matrix.cols; x++) {
            switch (matrix.type()) {
                case CV_8UC1: {
                    const unsigned char &entry = matrix.at<unsigned char>(y, x);
                    stream << entry;
                    break;
                }
                case CV_32FC3: {
                    const cv::Vec3f &entry = matrix.at<cv::Vec3f>(y, x);
                    stream << entry[0] << entry[1] << entry[2];
                    break;
                }
                default: {
                    throw QString("Unhandled matrix format %1!").arg(matrix.type());
                }
            }
        }
    }

}


