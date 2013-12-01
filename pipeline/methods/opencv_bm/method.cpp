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
    : QObject(parent), StereoMethod()
{
    // Default image width, used to compute optimal parameters
    imageWidth = 640;

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
    switch (type) {
        case OpenCV: {
            // OpenCV (the method does not really use presets)
            bm = cv::StereoBM(cv::StereoBM::BASIC_PRESET);
            break;
        }
        case StereoMatch: {
            // "Stereo match" example
            bm = cv::StereoBM();

            bm.state->preFilterCap = 31;
            bm.state->SADWindowSize = 9;
            bm.state->minDisparity = 0;
            bm.state->numberOfDisparities = ((imageWidth/8) + 15) & -16;
            bm.state->textureThreshold = 10;
            bm.state->uniquenessRatio = 15;
            bm.state->speckleWindowSize = 100;
            bm.state->speckleRange = 32;
            bm.state->disp12MaxDiff = 1;
            
            break;
        }
    };

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
    bm(tmpImg1, tmpImg2, tmpDisparity);
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
    bm = cv::StereoBM();
    
    storage["PreFilterType"] >> bm.state->preFilterType;
    storage["PreFilterSize"] >> bm.state->preFilterSize;
    storage["PreFilterCap"] >> bm.state->preFilterCap;
    
    storage["SADWindowSize"] >> bm.state->SADWindowSize;
    storage["MinDisparity"] >> bm.state->minDisparity;
    storage["NumDisparities"] >> bm.state->numberOfDisparities;

    storage["TextureThreshold"] >> bm.state->textureThreshold;
    storage["UniquenessRatio"] >> bm.state->uniquenessRatio;
    storage["SpeckleWindowSize"] >> bm.state->speckleWindowSize;
    storage["SpeckleRange"] >> bm.state->speckleRange;

    storage["TrySmallerWindows"] >> bm.state->trySmallerWindows;

    storage["Disp12MaxDiff"] >> bm.state->disp12MaxDiff;

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
    storage << "PreFilterType" << bm.state->preFilterType;
    storage << "PreFilterSize" << bm.state->preFilterSize;
    storage << "PreFilterCap" << bm.state->preFilterCap;
    
    storage << "SADWindowSize" << bm.state->SADWindowSize;
    storage << "MinDisparity" << bm.state->minDisparity;
    storage << "NumDisparities" << bm.state->numberOfDisparities;

    storage << "TextureThreshold" << bm.state->textureThreshold;
    storage << "UniquenessRatio" << bm.state->uniquenessRatio;
    storage << "SpeckleWindowSize" << bm.state->speckleWindowSize;
    storage << "SpeckleRange" << bm.state->speckleRange;

    storage << "TrySmallerWindows" << bm.state->trySmallerWindows;

    storage << "Disp12MaxDiff" << bm.state->disp12MaxDiff;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Pre-filter type
int Method::getPreFilterType () const
{
    return bm.state->preFilterType;
}

void Method::setPreFilterType (int newValue)
{
    // Validate
    if (newValue != CV_STEREO_BM_NORMALIZED_RESPONSE && newValue != CV_STEREO_BM_XSOBEL) {
        newValue = CV_STEREO_BM_NORMALIZED_RESPONSE;
    }

    setParameter(bm.state->preFilterType, newValue);
}

// Pre-filter size
int Method::getPreFilterSize () const
{
    return bm.state->preFilterSize;
}

void Method::setPreFilterSize (int newValue)
{
    // Validate
    newValue += !(newValue % 2); // Must be odd
    newValue = qBound(5, newValue, 255);

    setParameter(bm.state->preFilterSize, newValue);
}
    
// Pre-filter clipping
int Method::getPreFilterCap () const
{
    return bm.state->preFilterCap;
}

void Method::setPreFilterCap (int newValue)
{
    // Validate
    newValue = qBound(1, newValue, 63);

    setParameter(bm.state->preFilterCap, newValue);
}
        

// Sum-of-absolute difference window size
int Method::getSADWindowSize () const
{
    return bm.state->SADWindowSize;
}

void Method::setSADWindowSize (int newValue)
{
    // Validate
    newValue += !(newValue % 2); // Must be odd
    newValue = qBound(5, newValue, 255);

    setParameter(bm.state->SADWindowSize, newValue);
}
    
// Minimum disparity
int Method::getMinDisparity () const
{
    return bm.state->minDisparity;
}

void Method::setMinDisparity (int newValue)
{
    setParameter(bm.state->minDisparity, newValue);
}

// Number of disparity levels
int Method::getNumDisparities () const
{
    return bm.state->numberOfDisparities;
}

void Method::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 16.0) * 16; // Must be divisible by 16
    newValue = qMax(16, newValue);
    
    setParameter(bm.state->numberOfDisparities, newValue);
}
    
  
// Post-filtering texture threshold
int Method::getTextureThreshold () const
{
    return bm.state->textureThreshold;
}

void Method::setTextureThreshold (int newValue)
{
    setParameter(bm.state->textureThreshold, newValue);
}

// Uniqueness ratio; accept disparity d* only if:
//  SAD(d) >= SAD(d*)*(1 + uniquenessRatio/100.)
// for any d!) d* +/- 1 within the search range
int Method::getUniquenessRatio () const
{
    return bm.state->uniquenessRatio;
}

void Method::setUniquenessRatio (int newValue)
{
    setParameter(bm.state->uniquenessRatio, newValue);
}

// Disparity variantion window
int Method::getSpeckleWindowSize () const
{
    return bm.state->speckleWindowSize;
}

void Method::setSpeckleWindowSize (int newValue)
{
    setParameter(bm.state->speckleWindowSize, newValue);
}

// Acceptable range of variation in window    
int Method::getSpeckleRange () const
{
    return bm.state->speckleRange;
}

void Method::setSpeckleRange (int newValue)
{
    setParameter(bm.state->speckleRange, newValue);
}
 
// Whether to try smaller windows or not (more accurate results, but slower)
bool Method::getTrySmallerWindows () const
{
    return bm.state->trySmallerWindows;
}

void Method::setTrySmallerWindows (bool newValue)
{
    setParameter(bm.state->trySmallerWindows, (int)newValue);
}

// Disp12MaxDiff
int Method::getDisp12MaxDiff () const
{
    return bm.state->disp12MaxDiff;
}

void Method::setDisp12MaxDiff (int newValue)
{
    setParameter(bm.state->disp12MaxDiff, newValue);
}
