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

    centerRoiW = centerRoiH = -1;

    useStereoMethodThread = false;
    stereoDroppedFramesCounter = 0;

    imagePairSourceActive = true;
    rectificationActive = true;
    stereoMethodActive = true;
    reprojectionActive = true;

    connect(this, SIGNAL(inputImagesChanged()), this, SLOT(rectifyImages()));
    connect(this, SIGNAL(rectifiedImagesChanged()), this, SLOT(computeDisparityImage()));
    connect(this, SIGNAL(disparityImageChanged()), this, SLOT(reprojectDisparityImage()));
    connect(this, SIGNAL(reprojectedImageChanged()), this, SIGNAL(processingCompleted()));

    connect(this, SIGNAL(imagePairSourceStateChanged(bool)), this, SLOT(beginProcessing()));
    connect(this, SIGNAL(rectificationStateChanged(bool)), this, SLOT(rectifyImages()));
    connect(this, SIGNAL(stereoMethodStateChanged(bool)), this, SLOT(computeDisparityImage()));
    connect(this, SIGNAL(reprojectionStateChanged(bool)), this, SLOT(reprojectDisparityImage()));

    connect(this, SIGNAL(centerRoiChanged()), this, SLOT(computeDisparityImage()));

    // Create rectification
    setRectification(new StereoRectification(this));

    // Create reprojection
    setReprojection(new StereoReprojection(this));

    // Load plugins in default plugin path
    setPluginDirectory();
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
// *                         Plugin management                         *
// *********************************************************************
static void recursiveDirectoryScan (QDir dir, QStringList &files)
{
    // List all files in current directory
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach (QString fileName, dir.entryList()) {
        files.append(dir.absoluteFilePath(fileName));
    }

    // List all directories and recursively scan them
    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach (QString dirName, dir.entryList()) {
        recursiveDirectoryScan(dir.absoluteFilePath(dirName), files);
    }
}

void StereoPipeline::setPluginDirectory (const QString &newDirectory)
{
    // Clear old plugins
    foreach (PluginFactory *plugin, plugins) {
        delete plugin;
    }
    plugins.clear();

    // If path is not provided, use default, which can be overriden
    // by environment variable
    if (!newDirectory.isEmpty()) {
        pluginDirectory = QDir(newDirectory);
    } else {
        QByteArray pluginEnvVariable = qgetenv ("MVL_STEREO_TOOLBOX_PLUGIN_DIR");
        if (!pluginEnvVariable.isEmpty()) {
            pluginDirectory = QDir(pluginEnvVariable);
        } else {
            pluginDirectory = QDir(MVL_STEREO_PIPELINE_PLUGIN_DIR);
        }
    }
    
    // Set new directory
    qDebug() << "Plugin path:" << pluginDirectory.absolutePath();

    // Recursively scan for plugins
    QStringList files;
    recursiveDirectoryScan(pluginDirectory.absolutePath(), files);

    foreach (QString fileName, files) {
        // Make sure it is a library
        if (!QLibrary::isLibrary(fileName)) {
            continue;
        }

        // Load plugin       
        QPluginLoader loader(fileName);
        loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

        QObject *plugin = loader.instance();
        if (plugin) {
            PluginFactory *factory = qobject_cast<PluginFactory *>(plugin);
            if (factory) {
                factory->setParent(this);
                plugins.append(factory);
            } else {
                qDebug() << "Failed to cast plugged object to PluginFactory!";
            }
        } else {
            qDebug() << "Failed to load plugin:" << loader.errorString();            
        }
    }
}

QString StereoPipeline::getPluginDirectory () const
{
    return pluginDirectory.absolutePath();
}

const QList<PluginFactory *> StereoPipeline::getAvailablePlugins () const
{
    return plugins;
}


// *********************************************************************
// *                        GPU/CUDA management                        *
// *********************************************************************
int StereoPipeline::getNumberOfGpuDevices ()
{
#ifdef HAVE_OPENCV_GPU
    return cv::gpu::getCudaEnabledDeviceCount();
#else
    return 0;
#endif
}


void StereoPipeline::setGpuDevice (int dev)
{
#ifdef HAVE_OPENCV_GPU
    cv::gpu::setDevice(dev);
    cv::gpu::GpuMat mat(4, 4, CV_32FC1); // Create a dummy matrix to initialize GPU
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
// *                             Center ROI                            *
// *********************************************************************
cv::Size StereoPipeline::getCenterRoiSize () const
{
    return cv::Size(centerRoiW, centerRoiH);
}

void StereoPipeline::setCenterRoiSize (const cv::Size &size)
{
    if (size.width != centerRoiW || size.height != centerRoiH) {
        centerRoiW = size.width;
        centerRoiH = size.height;

        // Recompute center ROI
        recomputeCenterRoi();
    }
}

const cv::Rect &StereoPipeline::getCenterRoi () const
{
    return centerRoi;
}

void StereoPipeline::recomputeCenterRoi ()
{
    cv::Rect roi;

    if (centerRoiW == -1 || centerRoiH == -1) {
        // Center ROI not set
        roi = cv::Rect();
    } else {
        // Center ROI set; use only part of rectified images
        int roiW = qBound(0, centerRoiW, rectifiedImageL.cols);
        int roiH = qBound(0, centerRoiH, rectifiedImageL.rows);

        roi = cv::Rect((rectifiedImageL.cols - roiW)/2, (rectifiedImageL.rows - roiH)/2, roiW, roiH);
    }
    
    if (roi != centerRoi) {
        centerRoi = roi;
        emit centerRoiChanged();
    }
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
        disconnect(rectification, SIGNAL(stateChanged(bool)), this, SLOT(updateReprojectionMatrix()));
        if (rectification->parent() == this) {
            rectification->deleteLater(); // Schedule for deletion
        }
    }
    
    rectification = newRectification;
    if (!rectification->parent()) {
        rectification->setParent(this);
    }
    
    connect(rectification, SIGNAL(stateChanged(bool)), this, SLOT(rectifyImages()));
    connect(rectification, SIGNAL(stateChanged(bool)), this, SLOT(updateReprojectionMatrix()));

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
    // Make sure rectification is marked as active
    if (!rectificationActive) {
        emit processingCompleted();
        return;
    }
    
    // Make sure we have rectification object set
    if (!rectification) {
        emit error("Stereo rectification object not set!");
        return;
    }

    // Make sure input images are of same size
    if (inputImageL.cols != inputImageR.cols || inputImageL.rows != inputImageR.rows) {
        emit error("Input images do not have same dimensions!");
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
    // Recompute center ROI, since image size could have changed
    recomputeCenterRoi();
    
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
        // Start processing not processing already; otherwise drop
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
            if (centerRoi == cv::Rect()) {
                // Make sure disparity image is of correct size
                disparityImage.create(rectifiedImageL.rows, rectifiedImageL.cols, CV_8UC1);

                // Compute disparity
                stereoMethod->computeDisparityImage(rectifiedImageL, rectifiedImageR, disparityImage, disparityLevels);
            } else {
                // Apply ROI
                cv::Mat rectifiedRoiL = rectifiedImageL(centerRoi);
                cv::Mat rectifiedRoiR = rectifiedImageR(centerRoi);

                // Make sure disparity image is of correct size
                disparityImage.create(rectifiedRoiL.rows, rectifiedRoiL.cols, CV_8UC1);

                // Compute disparity
                stereoMethod->computeDisparityImage(rectifiedRoiL, rectifiedRoiR, disparityImage, disparityLevels);
            }
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
        disconnect(reprojection, SIGNAL(useGpuChanged(bool)), this, SLOT(reprojectDisparityImage()));
        if (reprojection->parent() == this) {
            reprojection->deleteLater(); // Schedule for deletion
        }
    }
    
    reprojection = newReprojection;
    if (!reprojection->parent()) {
        reprojection->setParent(this);
    }
    
    connect(reprojection, SIGNAL(useGpuChanged(bool)), this, SLOT(reprojectDisparityImage()));

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
    // Make sure reprojection is marked as active
    if (!reprojectionActive) {
        emit processingCompleted();
        return;
    }
    
    // Make sure we have reprojection object set
    if (!reprojection) {
        emit error("Stereo reprojection object not set!");
        return;
    }

    // Reproject
    try {
        QTime timer; timer.start();
        reprojection->reprojectStereoDisparity(disparityImage, reprojectedImage);
        reprojectionComputationTime = timer.elapsed();
    } catch (std::exception &e) {
        qWarning() << "Failed to reproject:" << QString::fromStdString(e.what());
        reprojectedImage = cv::Mat();
    }

    emit reprojectedImageChanged();
}
