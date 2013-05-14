#include "StereoMethodSemiGlobalBlockMatching.h"


StereoMethodSemiGlobalBlockMatching::StereoMethodSemiGlobalBlockMatching (QObject *parent)
    : StereoMethod(parent)
{

}

StereoMethodSemiGlobalBlockMatching::~StereoMethodSemiGlobalBlockMatching ()
{
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
    sgbm.minDisparity = newValue;
    emit parameterChanged();
}

// Number of diparity levels
int StereoMethodSemiGlobalBlockMatching::getNumDisparities () const
{
    return sgbm.numberOfDisparities;
}

void StereoMethodSemiGlobalBlockMatching::setNumDisparities (int newValue)
{
    sgbm.numberOfDisparities = newValue;
    emit parameterChanged();
}

// Sum-of-absolute difference window size
int StereoMethodSemiGlobalBlockMatching::getSADWindowSize () const
{
    return sgbm.SADWindowSize;
}

void StereoMethodSemiGlobalBlockMatching::setSADWindowSize (int newValue)
{
    sgbm.SADWindowSize = newValue;
    emit parameterChanged();
}

// Pre-filter clipping
int StereoMethodSemiGlobalBlockMatching::getPreFilterCap () const
{
    return sgbm.preFilterCap;
}

void StereoMethodSemiGlobalBlockMatching::setPreFilterCap (int newValue)
{
    sgbm.preFilterCap = newValue;
    emit parameterChanged();
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
    sgbm.uniquenessRatio = newValue;
    emit parameterChanged();
}

// P1
int StereoMethodSemiGlobalBlockMatching::getP1 () const
{
    return sgbm.P1;
}

void StereoMethodSemiGlobalBlockMatching::setP1 (int newValue)
{
    sgbm.P1 = newValue;
    emit parameterChanged();
}

// P2    
int StereoMethodSemiGlobalBlockMatching::getP2 () const
{
    return sgbm.P2;
}

void StereoMethodSemiGlobalBlockMatching::setP2 (int newValue)
{
    sgbm.P2 = newValue;
    emit parameterChanged();
}

// Disparity variantion window
int StereoMethodSemiGlobalBlockMatching::getSpeckleWindowSize () const
{
    return sgbm.speckleWindowSize;
}

void StereoMethodSemiGlobalBlockMatching::setSpeckleWindowSize (int newValue)
{
    sgbm.speckleWindowSize = newValue;
    emit parameterChanged();
}

// Acceptable range of variation in window    
int StereoMethodSemiGlobalBlockMatching::getSpeckleRange () const
{
    return sgbm.speckleRange;
}

void StereoMethodSemiGlobalBlockMatching::setSpeckleRange (int newValue)
{
    sgbm.speckleRange = newValue;
    emit parameterChanged();
}

// Disp12MaxDiff
int StereoMethodSemiGlobalBlockMatching::getDisp12MaxDiff () const
{
    return sgbm.disp12MaxDiff;
}

void StereoMethodSemiGlobalBlockMatching::setDisp12MaxDiff (int newValue)
{
    sgbm.disp12MaxDiff = newValue;
    emit parameterChanged();
}


// Whether to try smaller windows or not (more accurate results, but slower)
bool StereoMethodSemiGlobalBlockMatching::getFullDP () const
{
    return sgbm.fullDP;
}

void StereoMethodSemiGlobalBlockMatching::setFullDP (bool newValue)
{
    sgbm.fullDP = newValue;
    emit parameterChanged();
}

//
// Depth image computation
//
void StereoMethodSemiGlobalBlockMatching::computeDepthImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &depth)
{
    sgbm(img1, img2, depth);
}


