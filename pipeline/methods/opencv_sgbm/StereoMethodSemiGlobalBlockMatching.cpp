/*
 * OpenCV Semi-Global Block Matching: method
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
 
#include "StereoMethodSemiGlobalBlockMatching.h"
#include "StereoMethodSemiGlobalBlockMatchingConfigWidget.h"

#include <opencv2/imgproc/imgproc.hpp>


StereoMethodSemiGlobalBlockMatching::StereoMethodSemiGlobalBlockMatching (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "SGBM";
    usePreset(OpenCV);
}

StereoMethodSemiGlobalBlockMatching::~StereoMethodSemiGlobalBlockMatching ()
{
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
QWidget *StereoMethodSemiGlobalBlockMatching::createConfigWidget (QWidget *parent)
{
    return new StereoMethodSemiGlobalBlockMatchingConfigWidget(this, parent);
}


// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void StereoMethodSemiGlobalBlockMatching::usePreset (int type)
{
    switch (type) {
        case OpenCV: {
            // OpenCV
            sgbm = cv::StereoSGBM();
            sgbm.numberOfDisparities = ((imageWidth/8) + 15) & -16;
            break;
        }
        case StereoMatch: {
            // "Stereo match" example
            sgbm = cv::StereoSGBM();

            sgbm.preFilterCap = 63;
            sgbm.SADWindowSize = 3;

            sgbm.P1 = 8*imageChannels*sgbm.SADWindowSize*sgbm.SADWindowSize;
            sgbm.P2 = 32*imageChannels*sgbm.SADWindowSize*sgbm.SADWindowSize;
            sgbm.minDisparity = 0;
            sgbm.numberOfDisparities = ((imageWidth/8) + 15) & -16;
            sgbm.uniquenessRatio = 10;
            sgbm.speckleWindowSize = 100;
            sgbm.speckleRange = 32;
            sgbm.disp12MaxDiff = 1;
            
            break;
        }
    };

    emit parameterChanged();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void StereoMethodSemiGlobalBlockMatching::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    // Compute disparity image
    sgbm(img1, img2, tmpDisparity);

    // Normalize to output
    tmpDisparity.convertTo(disparity, CV_8U, 255/(sgbm.numberOfDisparities*16.));

    // Number of disparities
    numDisparities = getNumDisparities();
}


// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethodSemiGlobalBlockMatching::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethodSemiGlobalBlockMatching::loadParameters(storage);
    
    // Load parameters
    sgbm = cv::StereoSGBM();

    storage["MinDisparity"] >> sgbm.minDisparity;
    storage["NumDisparities"] >> sgbm.numberOfDisparities;
    storage["SADWindowSize"] >> sgbm.SADWindowSize;
    
    storage["PreFilterCap"] >> sgbm.preFilterCap;
    storage["UniquenessRatio"] >> sgbm.uniquenessRatio;

    storage["P1"] >> sgbm.P1;
    storage["P2"] >> sgbm.P2;

    storage["SpeckleWindowSize"] >> sgbm.speckleWindowSize;
    storage["SpeckleRange"] >> sgbm.speckleRange;

    storage["Disp12MaxDiff"] >> sgbm.disp12MaxDiff;

    storage["FullDP"] >> sgbm.fullDP;
    
    emit parameterChanged();
}

void StereoMethodSemiGlobalBlockMatching::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);

    // Save parameters
    storage << "MinDisparity" << sgbm.minDisparity;
    storage << "NumDisparities" << sgbm.numberOfDisparities;
    storage << "SADWindowSize" << sgbm.SADWindowSize;

    storage << "PreFilterCap" << sgbm.preFilterCap;
    storage << "UniquenessRatio" << sgbm.uniquenessRatio;

    storage << "P1" << sgbm.P1;
    storage << "P2" << sgbm.P2;

    storage << "SpeckleWindowSize" << sgbm.speckleWindowSize;
    storage << "SpeckleRange" << sgbm.speckleRange;

    storage << "Disp12MaxDiff" << sgbm.disp12MaxDiff;

    storage << "FullDP" << sgbm.fullDP;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Minimum disparity
int StereoMethodSemiGlobalBlockMatching::getMinDisparity () const
{
    return sgbm.minDisparity;
}

void StereoMethodSemiGlobalBlockMatching::setMinDisparity (int newValue)
{
    setParameter(sgbm.minDisparity, newValue);
}

// Number of diparity levels
int StereoMethodSemiGlobalBlockMatching::getNumDisparities () const
{
    return sgbm.numberOfDisparities;
}

void StereoMethodSemiGlobalBlockMatching::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 16.0) * 16; // Must be divisible by 16
    newValue = qMax(16, newValue);
    
    setParameter(sgbm.numberOfDisparities, newValue);
}

// Sum-of-absolute difference window size
int StereoMethodSemiGlobalBlockMatching::getSADWindowSize () const
{
    return sgbm.SADWindowSize;
}

void StereoMethodSemiGlobalBlockMatching::setSADWindowSize (int newValue)
{
    setParameter(sgbm.SADWindowSize, newValue);
}

// Pre-filter clipping
int StereoMethodSemiGlobalBlockMatching::getPreFilterCap () const
{
    return sgbm.preFilterCap;
}

void StereoMethodSemiGlobalBlockMatching::setPreFilterCap (int newValue)
{
    setParameter(sgbm.preFilterCap, newValue);
}


// Uniqueness ratio; accept disparity d* only if:
//  SAD(d) >= SAD(d*)*(1 + uniquenessRatio/100.)
// for any d!) d* +/- 1 within the search range
int StereoMethodSemiGlobalBlockMatching::getUniquenessRatio () const
{
    return sgbm.uniquenessRatio;
}

void StereoMethodSemiGlobalBlockMatching::setUniquenessRatio (int newValue)
{
    setParameter(sgbm.uniquenessRatio, newValue);
}

// P1
int StereoMethodSemiGlobalBlockMatching::getP1 () const
{
    return sgbm.P1;
}

void StereoMethodSemiGlobalBlockMatching::setP1 (int newValue)
{
    setParameter(sgbm.P1, newValue);
}

// P2    
int StereoMethodSemiGlobalBlockMatching::getP2 () const
{
    return sgbm.P2;
}

void StereoMethodSemiGlobalBlockMatching::setP2 (int newValue)
{
    setParameter(sgbm.P2, newValue);
}

// Disparity variantion window
int StereoMethodSemiGlobalBlockMatching::getSpeckleWindowSize () const
{
    return sgbm.speckleWindowSize;
}

void StereoMethodSemiGlobalBlockMatching::setSpeckleWindowSize (int newValue)
{
    setParameter(sgbm.speckleWindowSize, newValue);
}

// Acceptable range of variation in window    
int StereoMethodSemiGlobalBlockMatching::getSpeckleRange () const
{
    return sgbm.speckleRange;
}

void StereoMethodSemiGlobalBlockMatching::setSpeckleRange (int newValue)
{
    setParameter(sgbm.speckleRange, newValue);
}

// Disp12MaxDiff
int StereoMethodSemiGlobalBlockMatching::getDisp12MaxDiff () const
{
    return sgbm.disp12MaxDiff;
}

void StereoMethodSemiGlobalBlockMatching::setDisp12MaxDiff (int newValue)
{
    setParameter(sgbm.disp12MaxDiff, newValue);
}


// FullDP - double pass? If set to true, method becomes called "HH" in
// stereo match OpenCV example; otherwise, it is SGBM
bool StereoMethodSemiGlobalBlockMatching::getFullDP () const
{
    return sgbm.fullDP;
}

void StereoMethodSemiGlobalBlockMatching::setFullDP (bool newValue)
{
    setParameter(sgbm.fullDP, newValue);
}
