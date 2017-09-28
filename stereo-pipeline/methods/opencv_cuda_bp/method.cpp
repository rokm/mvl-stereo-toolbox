/*
 * OpenCV CUDA Belief Propagation: method
 * Copyright (C) 2013-2017 Rok Mandeljc
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

#include "method.h"
#include "method_widget.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvCudaBp {


Method::Method (QObject *parent)
    : QObject(parent), StereoMethod(),
      bp(cv::cuda::createStereoBeliefPropagation()),
      imageWidth(320), imageHeight(240)
{
}

Method::~Method ()
{
}


// *********************************************************************
// *                       StereoMethod interface                      *
// *********************************************************************
QString Method::getShortName () const
{
    return "CUDA_BP";
}

QWidget *Method::createConfigWidget (QWidget *parent)
{
    return new MethodWidget(this, parent);
}



// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void Method::usePreset (int type)
{
    QMutexLocker locker(&mutex);

    switch (type) {
        case OpenCVInit: {
            // OpenCV stock
            bp->setNumDisparities(64);
            bp->setNumIters(5);
            bp->setNumLevels(5);
            bp->setMaxDataTerm(10.0f);
            bp->setDataWeight(0.07f);
            bp->setMaxDiscTerm(1.7f);
            bp->setDiscSingleJump(1.0f);

            break;
        }
        case OpenCVRecommended: {
            // OpenCV recommended parameters estimation
            int ndisp, iters, levels;
            bp->estimateRecommendedParams(imageWidth, imageHeight, ndisp, iters, levels);

            bp->setNumDisparities(ndisp);
            bp->setNumIters(iters);
            bp->setNumLevels(levels);

            break;
        }
    };

    locker.unlock();
    emit parameterChanged();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void Method::computeDisparity (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    cv::cuda::GpuMat gpu_disp;

    // Store in case user wants to estimate optimal parameters
    imageWidth = img1.cols;
    imageHeight = img1.rows;

    if (1) {
        // Make sure that GPU matrices are destroyed as soon as they are
        // not needed anymore via scoping...
        cv::cuda::GpuMat gpu_img1(img1);
        cv::cuda::GpuMat gpu_img2(img2);

        // Compute disparity image
        QMutexLocker locker(&mutex);
        bp->compute(gpu_img1, gpu_img2, gpu_disp);
        locker.unlock();
    }

    // Download and return
    gpu_disp.download(tmpDisparity);
    tmpDisparity.convertTo(disparity, CV_32FC1);

    numDisparities = getNumDisparities();
}


// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void Method::loadParameters (const QString &filename)
{
    // Open storage
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::READ);
    if (!storage.isOpened()) {
        throw QString("Cannot open file \"%1\" for reading!").arg(filename);
    }

    // Validate data type
    QString dataType = QString::fromStdString(storage["DataType"]);
    if (dataType.compare("StereoMethodParameters")) {
        throw QString("Invalid stereo method parameters configuration!");
    }

    // Validate method name
    QString storedName = QString::fromStdString(storage["MethodName"]);
    if (storedName.compare(getShortName())) {
        throw QString("Invalid configuration for method \"%1\"!").arg(getShortName());
    }

    // Load parameters
    QMutexLocker locker(&mutex);

    bp->setNumDisparities((int)storage["NumDisparities"]);

    bp->setNumIters((int)storage["Iterations"]);
    bp->setNumLevels((int)storage["Levels"]);

    bp->setMaxDataTerm((double)storage["MaxDataTerm"]);
    bp->setDataWeight((double)storage["DataWeight"]);
    bp->setMaxDiscTerm((double)storage["MaxDiscTerm"]);
    bp->setDiscSingleJump((double)storage["DiscSingleJump"]);

    locker.unlock();

    emit parameterChanged();
}

void Method::saveParameters (const QString &filename) const
{
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::WRITE);
    if (!storage.isOpened()) {
        throw QString("Cannot open file \"%1\" for writing!").arg(filename);
    }

    // Data type
    storage << "DataType" << "StereoMethodParameters";

    // Store method name, so it can be validate upon loading
    storage << "MethodName" << getShortName().toStdString();

    storage << "NumDisparities" << bp->getNumDisparities();

    storage << "Iterations" << bp->getNumIters();
    storage << "Levels" << bp->getNumLevels();

    storage << "MaxDataTerm" << bp->getMaxDataTerm();
    storage << "DataWeight" << bp->getDataWeight();
    storage << "MaxDiscTerm" << bp->getMaxDiscTerm();
    storage << "DiscSingleJump" << bp->getDiscSingleJump();
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Number of disparities
int Method::getNumDisparities () const
{
    return bp->getNumDisparities();
}

void Method::setNumDisparities (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    bp->setNumDisparities(value);
    locker.unlock();

    emit parameterChanged();
}

// Number of iterations
int Method::getIterations () const
{
    return bp->getNumIters();
}

void Method::setIterations (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    bp->setNumIters(value);
    locker.unlock();

    emit parameterChanged();
}

// Levels
int Method::getLevels () const
{
    return bp->getNumLevels();
}

void Method::setLevels (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    bp->setNumLevels(value);
    locker.unlock();

    emit parameterChanged();
}

// Max data term
double Method::getMaxDataTerm () const
{
    return bp->getMaxDataTerm();
}

void Method::setMaxDataTerm (double value)
{
    // Set
    QMutexLocker locker(&mutex);
    bp->setMaxDataTerm(value);
    locker.unlock();

    emit parameterChanged();
}

// Data weight
double Method::getDataWeight () const
{
    return bp->getDataWeight();
}

void Method::setDataWeight (double value)
{
    // Set
    QMutexLocker locker(&mutex);
    bp->setDataWeight(value);
    locker.unlock();

    emit parameterChanged();
}

// Max discontinuity term
double Method::getMaxDiscTerm () const
{
    return bp->getMaxDiscTerm();
}

void Method::setMaxDiscTerm (double value)
{
    // Set
    QMutexLocker locker(&mutex);
    bp->setMaxDiscTerm(value);
    locker.unlock();

    emit parameterChanged();
}

// Single discontinuity jump
double Method::getDiscSingleJump () const
{
    return bp->getDiscSingleJump();
}

void Method::setDiscSingleJump (double value)
{
    // Set
    QMutexLocker locker(&mutex);
    bp->setDiscSingleJump(value);
    locker.unlock();

    emit parameterChanged();
}


} // StereoMethodOpenCvCudaBp
} // Pipeline
} // StereoToolbox
} // MVL
