/*
 * OpenCV Block Matching: method
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include "StereoMethodBlockMatching.h"
#include "StereoMethodBlockMatchingConfigWidget.h"

#include <opencv2/imgproc/imgproc.hpp>


StereoMethodBlockMatching::StereoMethodBlockMatching (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "BM";

    usePreset(OpenCVBasic);
}

StereoMethodBlockMatching::~StereoMethodBlockMatching ()
{
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
QWidget *StereoMethodBlockMatching::createConfigWidget (QWidget *parent)
{
    return new StereoMethodBlockMatchingConfigWidget(this, parent);
}


// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void StereoMethodBlockMatching::usePreset (int type)
{
    switch (type) {
        case OpenCVBasic: {
            // OpenCV basic
            bm = cv::StereoBM(cv::StereoBM::BASIC_PRESET);
            break;
        }
        case OpenCVFishEye: {
            // OpenCV fish eye
            bm = cv::StereoBM(cv::StereoBM::FISH_EYE_PRESET);
            break;
        }
        case OpenCVNarrow: {
            // OpenCV narrow
            bm = cv::StereoBM(cv::StereoBM::NARROW_PRESET);
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
void StereoMethodBlockMatching::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    // Convert to grayscale
    if (img1.channels() == 3) {
        cv::cvtColor(img1, tmpImg1, CV_RGB2GRAY);
    } else {
        tmpImg1 = img1;
    }

    if (img2.channels() == 3) {
        cv::cvtColor(img2, tmpImg2, CV_RGB2GRAY);
    } else {
        tmpImg2 = img2;
    }

    
    // Compute depth image
    bm(tmpImg1, tmpImg2, tmpDisparity);

    // Normalize to output
    tmpDisparity.convertTo(disparity, CV_8U, 255/(bm.state->numberOfDisparities*16.));

    // Number of disparities
    numDisparities = getNumDisparities();
}


// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethodBlockMatching::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
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

void StereoMethodBlockMatching::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);

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
int StereoMethodBlockMatching::getPreFilterType () const
{
    return bm.state->preFilterType;
}

void StereoMethodBlockMatching::setPreFilterType (int newValue)
{
    // Validate
    if (newValue != CV_STEREO_BM_NORMALIZED_RESPONSE && newValue != CV_STEREO_BM_XSOBEL) {
        newValue = CV_STEREO_BM_NORMALIZED_RESPONSE;
    }

    setParameter(bm.state->preFilterType, newValue);
}

// Pre-filter size
int StereoMethodBlockMatching::getPreFilterSize () const
{
    return bm.state->preFilterSize;
}

void StereoMethodBlockMatching::setPreFilterSize (int newValue)
{
    // Validate
    newValue += !(newValue % 2); // Must be odd
    newValue = qBound(5, newValue, 255);

    setParameter(bm.state->preFilterSize, newValue);
}
    
// Pre-filter clipping
int StereoMethodBlockMatching::getPreFilterCap () const
{
    return bm.state->preFilterCap;
}

void StereoMethodBlockMatching::setPreFilterCap (int newValue)
{
    // Validate
    newValue = qBound(1, newValue, 63);

    setParameter(bm.state->preFilterCap, newValue);
}
        

// Sum-of-absolute difference window size
int StereoMethodBlockMatching::getSADWindowSize () const
{
    return bm.state->SADWindowSize;
}

void StereoMethodBlockMatching::setSADWindowSize (int newValue)
{
    // Validate
    newValue += !(newValue % 2); // Must be odd
    newValue = qBound(5, newValue, 255);

    setParameter(bm.state->SADWindowSize, newValue);
}
    
// Minimum disparity
int StereoMethodBlockMatching::getMinDisparity () const
{
    return bm.state->minDisparity;
}

void StereoMethodBlockMatching::setMinDisparity (int newValue)
{
    setParameter(bm.state->minDisparity, newValue);
}

// Number of disparity levels
int StereoMethodBlockMatching::getNumDisparities () const
{
    return bm.state->numberOfDisparities;
}

void StereoMethodBlockMatching::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 16.0) * 16; // Must be divisible by 16
    newValue = qMax(16, newValue);
    
    setParameter(bm.state->numberOfDisparities, newValue);
}
    
  
// Post-filtering texture threshold
int StereoMethodBlockMatching::getTextureThreshold () const
{
    return bm.state->textureThreshold;
}

void StereoMethodBlockMatching::setTextureThreshold (int newValue)
{
    setParameter(bm.state->textureThreshold, newValue);
}

// Uniqueness ratio; accept disparity d* only if:
//  SAD(d) >= SAD(d*)*(1 + uniquenessRatio/100.)
// for any d!) d* +/- 1 within the search range
int StereoMethodBlockMatching::getUniquenessRatio () const
{
    return bm.state->uniquenessRatio;
}

void StereoMethodBlockMatching::setUniquenessRatio (int newValue)
{
    setParameter(bm.state->uniquenessRatio, newValue);
}

// Disparity variantion window
int StereoMethodBlockMatching::getSpeckleWindowSize () const
{
    return bm.state->speckleWindowSize;
}

void StereoMethodBlockMatching::setSpeckleWindowSize (int newValue)
{
    setParameter(bm.state->speckleWindowSize, newValue);
}

// Acceptable range of variation in window    
int StereoMethodBlockMatching::getSpeckleRange () const
{
    return bm.state->speckleRange;
}

void StereoMethodBlockMatching::setSpeckleRange (int newValue)
{
    setParameter(bm.state->speckleRange, newValue);
}
 
// Whether to try smaller windows or not (more accurate results, but slower)
bool StereoMethodBlockMatching::getTrySmallerWindows () const
{
    return bm.state->trySmallerWindows;
}

void StereoMethodBlockMatching::setTrySmallerWindows (bool newValue)
{
    setParameter(bm.state->trySmallerWindows, (int)newValue);
}

// Disp12MaxDiff
int StereoMethodBlockMatching::getDisp12MaxDiff () const
{
    return bm.state->disp12MaxDiff;
}

void StereoMethodBlockMatching::setDisp12MaxDiff (int newValue)
{
    setParameter(bm.state->disp12MaxDiff, newValue);
}
