/*
 * OpenCV Block Matching: method
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

#include <stereo-pipeline/exception.h>

#include <opencv2/imgproc.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvBm {


Method::Method (QObject *parent)
    : QObject(parent), StereoMethod(),
      bm(cv::StereoBM::create()),
      imageWidth(640)
{
    usePreset(OpenCV);
}

Method::~Method ()
{
}


// *********************************************************************
// *                       StereoMethod interface                      *
// *********************************************************************
QString Method::getShortName () const
{
    return "BM";
}

QWidget *Method::createConfigWidget (QWidget *parent)
{
    return new MethodWidget(this, parent);
}



// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void Method::usePreset (int preset)
{
    QMutexLocker locker(&mutex);

    switch (preset) {
        case OpenCV: {
            // Default OpenCV settings
            bm->setPreFilterType(cv::StereoBM::PREFILTER_XSOBEL);
            bm->setPreFilterSize(9);
            bm->setPreFilterCap(31);
            bm->setBlockSize(21);
            bm->setMinDisparity(0);
            bm->setNumDisparities(64);
            bm->setTextureThreshold(10);
            bm->setUniquenessRatio(15);
            bm->setSpeckleWindowSize(0);
            bm->setSpeckleRange(0);
            bm->setDisp12MaxDiff(-1);

            break;
        }
        case StereoMatch: {
            // "Stereo match" example
            bm->setPreFilterType(cv::StereoBM::PREFILTER_XSOBEL);
            bm->setPreFilterSize(9);
            bm->setPreFilterCap(31);
            bm->setBlockSize(9);
            bm->setMinDisparity(0);
            bm->setNumDisparities(((imageWidth/8) + 15) & -16);
            bm->setTextureThreshold(10);
            bm->setUniquenessRatio(15);
            bm->setSpeckleWindowSize(100);
            bm->setSpeckleRange(32);
            bm->setDisp12MaxDiff(1);

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
    // Convert to grayscale
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

    // Store in case user wants to compute optimal parameters
    imageWidth = img1.cols;

    // Compute disparity image
    tmpDisparity.create(img1.rows, img1.cols, CV_16SC1);

    QMutexLocker locker(&mutex);
    bm->compute(tmpImg1, tmpImg2, tmpDisparity);
    locker.unlock();

    // Normalize to output float format
    switch (tmpDisparity.type()) {
        case CV_16SC1: {
            tmpDisparity.convertTo(disparity, CV_32F, 1/16.0);
            break;
        }
        default: {
            tmpDisparity.convertTo(disparity, CV_32F);
        }
    }

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
        throw Exception(QStringLiteral("Cannot open file '%1' for reading!").arg(filename));
    }

    // Validate data type
    QString dataType = QString::fromStdString(storage["DataType"]);
    if (dataType.compare("StereoMethodParameters")) {
        throw Exception(QStringLiteral("Invalid stereo method parameters configuration!"));
    }

    // Validate method name
    QString storedName = QString::fromStdString(storage["MethodName"]);
    if (storedName.compare(getShortName())) {
        throw Exception(QStringLiteral("Invalid configuration for method '%1'!").arg(getShortName()));
    }

    // Load parameters
    QMutexLocker locker(&mutex);

    bm->setPreFilterType((int)storage["PreFilterType"]);
    bm->setPreFilterSize((int)storage["PreFilterSize"]);
    bm->setPreFilterCap((int)storage["PreFilterCap"]);

    bm->setBlockSize((int)storage["SADWindowSize"]);
    bm->setMinDisparity((int)storage["MinDisparity"]);
    bm->setNumDisparities((int)storage["NumDisparities"]);

    bm->setTextureThreshold((int)storage["TextureThreshold"]);
    bm->setUniquenessRatio((int)storage["UniquenessRatio"]);
    bm->setSpeckleWindowSize((int)storage["SpeckleWindowSize"]);
    bm->setSpeckleRange((int)storage["SpeckleRange"]);

    bm->setDisp12MaxDiff((int)storage["Disp12MaxDiff"]);

    locker.unlock();

    emit parameterChanged();
}

void Method::saveParameters (const QString &filename) const
{
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::WRITE);
    if (!storage.isOpened()) {
        throw Exception(QStringLiteral("Cannot open file '%1' for writing!").arg(filename));
    }

    // Data type
    storage << "DataType" << "StereoMethodParameters";

    // Store method name, so it can be validate upon loading
    storage << "MethodName" << getShortName().toStdString();

    // Save parameters
    storage << "PreFilterType" << bm->getPreFilterType();
    storage << "PreFilterSize" << bm->getPreFilterSize();
    storage << "PreFilterCap" << bm->getPreFilterCap();

    storage << "SADWindowSize" << bm->getBlockSize();
    storage << "MinDisparity" << bm->getMinDisparity();
    storage << "NumDisparities" << bm->getNumDisparities();

    storage << "TextureThreshold" << bm->getTextureThreshold();
    storage << "UniquenessRatio" << bm->getUniquenessRatio();
    storage << "SpeckleWindowSize" << bm->getSpeckleWindowSize();
    storage << "SpeckleRange" << bm->getSpeckleRange();

    storage << "Disp12MaxDiff" << bm->getDisp12MaxDiff();
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Pre-filter type
int Method::getPreFilterType () const
{
    return bm->getPreFilterType();
}

void Method::setPreFilterType (int value)
{
    // Validate
    if (value != cv::StereoBM::PREFILTER_NORMALIZED_RESPONSE && value != cv::StereoBM::PREFILTER_XSOBEL) {
        value = cv::StereoBM::PREFILTER_NORMALIZED_RESPONSE;
    }

    // Set
    QMutexLocker locker(&mutex);
    bm->setPreFilterType(value);
    locker.unlock();

    emit parameterChanged();
}

// Pre-filter size
int Method::getPreFilterSize () const
{
    return bm->getPreFilterSize();
}

void Method::setPreFilterSize (int value)
{
    // Validate
    value += !(value % 2); // Must be odd
    value = qBound(5, value, 255);

    // Set
    QMutexLocker locker(&mutex);
    bm->setPreFilterSize(value);
    locker.unlock();

    emit parameterChanged();
}

// Pre-filter clipping
int Method::getPreFilterCap () const
{
    return bm->getPreFilterCap();
}

void Method::setPreFilterCap (int value)
{
    // Validate
    value = qBound(1, value, 63);

    // Set
    QMutexLocker locker(&mutex);
    bm->setPreFilterCap(value);
    locker.unlock();

    emit parameterChanged();
}


// Sum-of-absolute difference window size
int Method::getSADWindowSize () const
{
    return bm->getBlockSize();
}

void Method::setSADWindowSize (int value)
{
    // Validate
    value += !(value % 2); // Must be odd
    value = qBound(5, value, 255);

    // Set
    QMutexLocker locker(&mutex);
    bm->setBlockSize(value);
    locker.unlock();

    emit parameterChanged();
}

// Minimum disparity
int Method::getMinDisparity () const
{
    return bm->getMinDisparity();
}

void Method::setMinDisparity (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setMinDisparity(value);
    locker.unlock();

    emit parameterChanged();
}

// Number of disparity levels
int Method::getNumDisparities () const
{
    return bm->getNumDisparities();
}

void Method::setNumDisparities (int value)
{
    // Validate
    value = qRound(value / 16.0) * 16; // Must be divisible by 16
    value = qMax(16, value);

    // Set
    QMutexLocker locker(&mutex);
    bm->setNumDisparities(value);
    locker.unlock();

    emit parameterChanged();
}


// Post-filtering texture threshold
int Method::getTextureThreshold () const
{
    return bm->getTextureThreshold();
}

void Method::setTextureThreshold (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setTextureThreshold(value);
    locker.unlock();

    emit parameterChanged();
}

// Uniqueness ratio; accept disparity d* only if:
//  SAD(d) >= SAD(d*)*(1 + uniquenessRatio/100.)
// for any d!) d* +/- 1 within the search range
int Method::getUniquenessRatio () const
{
    return bm->getUniquenessRatio();
}

void Method::setUniquenessRatio (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setUniquenessRatio(value);
    locker.unlock();

    emit parameterChanged();
}

// Disparity variantion window
int Method::getSpeckleWindowSize () const
{
    return bm->getSpeckleWindowSize();
}

void Method::setSpeckleWindowSize (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setSpeckleWindowSize(value);
    locker.unlock();

    emit parameterChanged();
}

// Acceptable range of variation in window
int Method::getSpeckleRange () const
{
    return bm->getSpeckleRange();
}

void Method::setSpeckleRange (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setSpeckleRange(value);
    locker.unlock();

    emit parameterChanged();
}

// Disp12MaxDiff
int Method::getDisp12MaxDiff () const
{
    return bm->getDisp12MaxDiff();
}

void Method::setDisp12MaxDiff (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setDisp12MaxDiff(value);
    locker.unlock();

    emit parameterChanged();
}


} // StereoMethodOpenCvBm
} // Pipeline
} // StereoToolbox
} // MVL
