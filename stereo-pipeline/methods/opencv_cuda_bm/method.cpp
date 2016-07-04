/*
 * OpenCV CUDA Block Matching: method
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

#include "method.h"
#include "method_widget.h"

#include <opencv2/imgproc.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvCudaBm {


Method::Method (QObject *parent)
    : QObject(parent), StereoMethod(),
      bm(cv::cuda::createStereoBM())
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
    return "CUDA_BM";
}

QWidget *Method::createConfigWidget (QWidget *parent)
{
    return new MethodWidget(this, parent);
}



// *********************************************************************
// *                             Defaults                              *
// *********************************************************************
void Method::resetToDefaults ()
{
    QMutexLocker locker(&mutex);

    bm->setNumDisparities(64);
    bm->setBlockSize(19);
    bm->setPreFilterType(0);
    bm->setPreFilterCap(31);
    bm->setTextureThreshold(3);

    locker.unlock();

    emit parameterChanged();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void Method::computeDisparity (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    cv::cuda::GpuMat gpu_img1, gpu_img2, gpu_disp;

    // Convert to float grayscale
    if (img1.channels() == 3) {
        cv::cvtColor(img1, tmpImg1, cv::COLOR_BGR2GRAY);
    } else {
        tmpImg1 = img1;
    }

    if (img2.channels() == 3) {
        cv::cvtColor(img2, tmpImg2, cv::COLOR_BGR2GRAY);
    } else {
        tmpImg2 = img2;
    }

    // Upload to GPU
    gpu_img1.upload(tmpImg1);
    gpu_img2.upload(tmpImg2);

    // Compute disparity image
    QMutexLocker locker(&mutex);
    bm->compute(gpu_img1, gpu_img2, gpu_disp);
    locker.unlock();

    // Download and convert
    gpu_disp.download(tmpDisparity);
    tmpDisparity.convertTo(disparity, CV_32F);

    // Number of disparities
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

    bm->setPreFilterType((int)storage["PreFilterType"]);
    bm->setPreFilterCap((int)storage["PreFilterCap"]);
    bm->setNumDisparities((int)storage["NumDisparities"]);
    bm->setBlockSize((int)storage["WindowSize"]);
    bm->setTextureThreshold((float)storage["AverageTextureThreshold"]);

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

    // Save parameters
    storage << "PreFilterType" << bm->getPreFilterType();
    storage << "PreFilterCap" << bm->getPreFilterCap();
    storage << "NumDisparities" << bm->getNumDisparities();
    storage << "WindowSize" << bm->getBlockSize();
    storage << "AverageTextureThreshold" << bm->getTextureThreshold();
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Pre-filter type
int Method::getPreFilterType () const
{
    return bm->getPreFilterType();
}

void Method::setPreFilterType (int newValue)
{
    // Validate
    if (newValue != cv::StereoBM::PREFILTER_NORMALIZED_RESPONSE && newValue != cv::StereoBM::PREFILTER_XSOBEL) {
        newValue = cv::StereoBM::PREFILTER_NORMALIZED_RESPONSE;
    }

    // Set
    QMutexLocker locker(&mutex);
    bm->setPreFilterType(newValue);
    locker.unlock();

    emit parameterChanged();
}

// Pre-filter cap
int Method::getPreFilterCap () const
{
    return bm->getPreFilterCap();
}

void Method::setPreFilterCap (int newValue)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setPreFilterCap(newValue);
    locker.unlock();

    emit parameterChanged();
}

// Number of disparities
int Method::getNumDisparities () const
{
    return bm->getNumDisparities();
}

void Method::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 8.0) * 8; // Must be divisible by 8
    newValue = qMax(8, newValue);

    // Set
    QMutexLocker locker(&mutex);
    bm->setNumDisparities(newValue);
    locker.unlock();

    emit parameterChanged();
}

// Window size
int Method::getWindowSize () const
{
    return bm->getBlockSize();
}

void Method::setWindowSize (int newValue)
{
    // Validate
    if (newValue % 2 == 0) {
        newValue++; // Must be odd number
    }

    // Set
    QMutexLocker locker(&mutex);
    bm->setBlockSize(newValue);
    locker.unlock();

    emit parameterChanged();
}


// Average texture threshold
double Method::getAverageTextureThreshold () const
{
    return bm->getTextureThreshold();
}

void Method::setAverageTextureThreshold (double newValue)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setTextureThreshold(newValue);
    locker.unlock();

    emit parameterChanged();
}


} // StereoMethodOpenCvCudaBm
} // Pipeline
} // StereoToolbox
} // MVL
