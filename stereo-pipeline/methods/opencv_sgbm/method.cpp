/*
 * OpenCV Semi-Global Block Matching: method
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
namespace StereoMethodOpenCvSgbm {


Method::Method (QObject *parent)
    : QObject(parent), StereoMethod(),
      sgbm(cv::StereoSGBM::create(0, 16, 3)),
      imageWidth(640), imageChannels(1)
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
    return "SGBM";
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
        case OpenCV: {
            // Default OpenCV settings
            sgbm->setPreFilterCap(0);
            sgbm->setBlockSize(0);

            sgbm->setP1(0);
            sgbm->setP2(0);
            sgbm->setMinDisparity(0);
            sgbm->setNumDisparities(64);
            sgbm->setUniquenessRatio(0);
            sgbm->setSpeckleWindowSize(0);
            sgbm->setSpeckleRange(0);
            sgbm->setDisp12MaxDiff(-1);
            sgbm->setMode(cv::StereoSGBM::MODE_SGBM);

            break;
        }
        case StereoMatch: {
            // "Stereo match" example
            sgbm->setPreFilterCap(63);
            sgbm->setBlockSize(3);

            sgbm->setP1(8*imageChannels*3*3);
            sgbm->setP2(32*imageChannels*3*3);
            sgbm->setMinDisparity(0);
            sgbm->setNumDisparities(((imageWidth/8) + 15) & -16);
            sgbm->setUniquenessRatio(10);
            sgbm->setSpeckleWindowSize(100);
            sgbm->setSpeckleRange(32);
            sgbm->setDisp12MaxDiff(1);
            sgbm->setMode(cv::StereoSGBM::MODE_SGBM);

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
    // Store in case user wants to compute optimal parameters
    imageWidth = img1.cols;
    imageChannels = img1.channels();

    // Compute disparity image
    tmpDisparity.create(img1.rows, img1.cols, CV_16SC1);

    QMutexLocker locker(&mutex);
    sgbm->compute(img1, img2, tmpDisparity);
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

    sgbm->setMinDisparity((int)storage["MinDisparity"]);
    sgbm->setNumDisparities((int)storage["NumDisparities"]);
    sgbm->setBlockSize((int)storage["SADWindowSize"]);

    sgbm->setPreFilterCap((int)storage["PreFilterCap"]);
    sgbm->setUniquenessRatio((int)storage["UniquenessRatio"]);

    sgbm->setP1((int)storage["P1"]);
    sgbm->setP2((int)storage["P2"]);

    sgbm->setSpeckleWindowSize((int)storage["SpeckleWindowSize"]);
    sgbm->setSpeckleRange((int)storage["SpeckleRange"]);

    sgbm->setDisp12MaxDiff((int)storage["Disp12MaxDiff"]);

    sgbm->setMode((int)storage["Mode"]);

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
    storage << "MinDisparity" << sgbm->getMinDisparity();
    storage << "NumDisparities" << sgbm->getNumDisparities();
    storage << "SADWindowSize" << sgbm->getBlockSize();

    storage << "PreFilterCap" << sgbm->getPreFilterCap();
    storage << "UniquenessRatio" << sgbm->getUniquenessRatio();

    storage << "P1" << sgbm->getP1();
    storage << "P2" << sgbm->getP2();

    storage << "SpeckleWindowSize" << sgbm->getSpeckleWindowSize();
    storage << "SpeckleRange" << sgbm->getSpeckleRange();

    storage << "Disp12MaxDiff" << sgbm->getDisp12MaxDiff();

    storage << "Mode" << sgbm->getMode();
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Minimum disparity
int Method::getMinDisparity () const
{
    return sgbm->getMinDisparity();
}

void Method::setMinDisparity (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    sgbm->setMinDisparity(value);
    locker.unlock();

    emit parameterChanged();
}

// Number of diparity levels
int Method::getNumDisparities () const
{
    return sgbm->getNumDisparities();
}

void Method::setNumDisparities (int value)
{
    // Validate
    value = qRound(value / 16.0) * 16; // Must be divisible by 16
    value = qMax(16, value);

    // Set
    QMutexLocker locker(&mutex);
    sgbm->setNumDisparities(value);
    locker.unlock();

    emit parameterChanged();
}

// Sum-of-absolute difference window size
int Method::getSADWindowSize () const
{
    return sgbm->getBlockSize();
}

void Method::setSADWindowSize (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    sgbm->setBlockSize(value);
    locker.unlock();

    emit parameterChanged();
}

// Pre-filter clipping
int Method::getPreFilterCap () const
{
    return sgbm->getPreFilterCap();
}

void Method::setPreFilterCap (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    sgbm->setPreFilterCap(value);
    locker.unlock();

    emit parameterChanged();
}


// Uniqueness ratio; accept disparity d* only if:
//  SAD(d) >= SAD(d*)*(1 + uniquenessRatio/100.)
// for any d!) d* +/- 1 within the search range
int Method::getUniquenessRatio () const
{
    return sgbm->getUniquenessRatio();
}

void Method::setUniquenessRatio (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    sgbm->setUniquenessRatio(value);
    locker.unlock();

    emit parameterChanged();
}

// P1
int Method::getP1 () const
{
    return sgbm->getP1();
}

void Method::setP1 (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    sgbm->setP1(value);
    locker.unlock();

    emit parameterChanged();
}

// P2
int Method::getP2 () const
{
    return sgbm->getP2();
}

void Method::setP2 (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    sgbm->setP2(value);
    locker.unlock();

    emit parameterChanged();
}

// Disparity variantion window
int Method::getSpeckleWindowSize () const
{
    return sgbm->getSpeckleWindowSize();
}

void Method::setSpeckleWindowSize (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    sgbm->setSpeckleWindowSize(value);
    locker.unlock();

    emit parameterChanged();
}

// Acceptable range of variation in window
int Method::getSpeckleRange () const
{
    return sgbm->getSpeckleRange();
}

void Method::setSpeckleRange (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    sgbm->setSpeckleRange(value);
    locker.unlock();

    emit parameterChanged();
}

// Disp12MaxDiff
int Method::getDisp12MaxDiff () const
{
    return sgbm->getDisp12MaxDiff();
}

void Method::setDisp12MaxDiff (int value)
{
    // Set
    QMutexLocker locker(&mutex);
    sgbm->setDisp12MaxDiff(value);
    locker.unlock();

    emit parameterChanged();
}


// Mode: single-pass (5 directions) or full (8 directions)
int Method::getMode () const
{
    return sgbm->getMode();
}

void Method::setMode (int value)
{
    // Validate
    if (value != cv::StereoSGBM::MODE_SGBM && value != cv::StereoSGBM::MODE_HH) {
        value = cv::StereoSGBM::MODE_SGBM;
    }

    // Set
    QMutexLocker locker(&mutex);
    sgbm->setMode(value);
    locker.unlock();

    emit parameterChanged();
}


} // StereoMethodOpenCvSgbm
} // Pipeline
} // StereoToolbox
} // MVL
