/*
 * OpenCV Block Matching: method
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

#include "method.h"
#include "method_widget.h"

#include <opencv2/imgproc/imgproc.hpp>

using namespace StereoMethodBlockMatching;


Method::Method (QObject *parent)
    : QObject(parent), StereoMethod(),
      bm(cv::createStereoBM()),
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
void Method::usePreset (int type)
{
    QMutexLocker locker(&mutex);

    switch (type) {
        case OpenCV: {
            // OpenCV 
            break;
        }
        case StereoMatch: {
            // "Stereo match" example
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
void Method::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
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

    // Normalize to output
    if (tmpDisparity.type() == CV_16SC1) {
        tmpDisparity.convertTo(disparity, CV_8U, 1/16.0);
    } else if (tmpDisparity.type() == CV_32FC1) {
        tmpDisparity.convertTo(disparity, CV_8U);
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
        throw QString("Cannot open file \"%1\" for writing!").arg(filename);
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

// Pre-filter size
int Method::getPreFilterSize () const
{
    return bm->getPreFilterSize();
}

void Method::setPreFilterSize (int newValue)
{
    // Validate
    newValue += !(newValue % 2); // Must be odd
    newValue = qBound(5, newValue, 255);

    // Set
    QMutexLocker locker(&mutex);
    bm->setPreFilterSize(newValue);
    locker.unlock();
        
    emit parameterChanged();
}

// Pre-filter clipping
int Method::getPreFilterCap () const
{
    return bm->getPreFilterCap();
}

void Method::setPreFilterCap (int newValue)
{
    // Validate
    newValue = qBound(1, newValue, 63);

    // Set
    QMutexLocker locker(&mutex);
    bm->setPreFilterCap(newValue);
    locker.unlock();
        
    emit parameterChanged();
}


// Sum-of-absolute difference window size
int Method::getSADWindowSize () const
{
    return bm->getBlockSize();
}

void Method::setSADWindowSize (int newValue)
{
    // Validate
    newValue += !(newValue % 2); // Must be odd
    newValue = qBound(5, newValue, 255);

    // Set
    QMutexLocker locker(&mutex);
    bm->setBlockSize(newValue);
    locker.unlock();
        
    emit parameterChanged();
}

// Minimum disparity
int Method::getMinDisparity () const
{
    return bm->getMinDisparity();
}

void Method::setMinDisparity (int newValue)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setMinDisparity(newValue);
    locker.unlock();
        
    emit parameterChanged();
}

// Number of disparity levels
int Method::getNumDisparities () const
{
    return bm->getNumDisparities();
}

void Method::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 16.0) * 16; // Must be divisible by 16
    newValue = qMax(16, newValue);

    // Set
    QMutexLocker locker(&mutex);
    bm->setNumDisparities(newValue);
    locker.unlock();
        
    emit parameterChanged();
}


// Post-filtering texture threshold
int Method::getTextureThreshold () const
{
    return bm->getTextureThreshold();
}

void Method::setTextureThreshold (int newValue)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setTextureThreshold(newValue);
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

void Method::setUniquenessRatio (int newValue)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setUniquenessRatio(newValue);
    locker.unlock();
        
    emit parameterChanged();
}

// Disparity variantion window
int Method::getSpeckleWindowSize () const
{
    return bm->getSpeckleWindowSize();
}

void Method::setSpeckleWindowSize (int newValue)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setSpeckleWindowSize(newValue);
    locker.unlock();
        
    emit parameterChanged();
}

// Acceptable range of variation in window
int Method::getSpeckleRange () const
{
    return bm->getSpeckleRange();
}

void Method::setSpeckleRange (int newValue)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setSpeckleRange(newValue);
    locker.unlock();
        
    emit parameterChanged();
}

// Disp12MaxDiff
int Method::getDisp12MaxDiff () const
{
    return bm->getDisp12MaxDiff();
}

void Method::setDisp12MaxDiff (int newValue)
{
    // Set
    QMutexLocker locker(&mutex);
    bm->setDisp12MaxDiff(newValue);
    locker.unlock();
        
    emit parameterChanged();
}
