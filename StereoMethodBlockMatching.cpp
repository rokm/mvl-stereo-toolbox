#include "StereoMethodBlockMatching.h"


StereoMethodBlockMatching::StereoMethodBlockMatching (QObject *parent)
    : StereoMethod(parent)
{

}

StereoMethodBlockMatching::~StereoMethodBlockMatching ()
{
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
    if (bm.state->preFilterType != newValue) {
        bm.state->preFilterType = newValue;
        emit parameterChanged();
    }
}

// Pre-filter size
int StereoMethodBlockMatching::getPreFilterSize () const
{
    return bm.state->preFilterSize;
}

void StereoMethodBlockMatching::setPreFilterSize (int newValue)
{
    if (bm.state->preFilterSize != newValue) {
        bm.state->preFilterSize = newValue;
        emit parameterChanged();
    }
}
    
// Pre-filter clipping
int StereoMethodBlockMatching::getPreFilterCap () const
{
    return bm.state->preFilterCap;
}

void StereoMethodBlockMatching::setPreFilterCap (int newValue)
{
    if (bm.state->preFilterCap != newValue) {
        bm.state->preFilterCap = newValue;
        emit parameterChanged();
    }
}
        

// Sum-of-absolute difference window size
int StereoMethodBlockMatching::getSADWindowSize () const
{
    return bm.state->SADWindowSize;
}

void StereoMethodBlockMatching::setSADWindowSize (int newValue)
{
    if (bm.state->SADWindowSize != newValue) {
        bm.state->SADWindowSize = newValue;
        emit parameterChanged();
    }
}
    
// Minimum disparity
int StereoMethodBlockMatching::getMinDisparity () const
{
    return bm.state->minDisparity;
}

void StereoMethodBlockMatching::setMinDisparity (int newValue)
{
    if (bm.state->minDisparity != newValue) {
        bm.state->minDisparity = newValue;
        emit parameterChanged();
    }
}

// Number of disparity levels
int StereoMethodBlockMatching::getNumDisparities () const
{
    return bm.state->numberOfDisparities;
}

void StereoMethodBlockMatching::setNumDisparities (int newValue)
{
    if (bm.state->numberOfDisparities != newValue) {
        bm.state->numberOfDisparities = newValue;
        emit parameterChanged();
    }
}
    
  
// Post-filtering texture threshold
int StereoMethodBlockMatching::getTextureThreshold () const
{
    return bm.state->textureThreshold;
}

void StereoMethodBlockMatching::setTextureThreshold (int newValue)
{
    if (bm.state->textureThreshold != newValue) {
        bm.state->textureThreshold = newValue;
        emit parameterChanged();
    }
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
    if (bm.state->uniquenessRatio != newValue) {
        bm.state->uniquenessRatio = newValue;
        emit parameterChanged();
    }
}

// Disparity variantion window
int StereoMethodBlockMatching::getSpeckleWindowSize () const
{
    return bm.state->speckleWindowSize;
}

void StereoMethodBlockMatching::setSpeckleWindowSize (int newValue)
{
    if (bm.state->speckleWindowSize != newValue) {
        bm.state->speckleWindowSize = newValue;
        emit parameterChanged();
    }
}

// Acceptable range of variation in window    
int StereoMethodBlockMatching::getSpeckleRange () const
{
    return bm.state->speckleRange;
}

void StereoMethodBlockMatching::setSpeckleRange (int newValue)
{
    if (bm.state->speckleRange != newValue) {
        bm.state->speckleRange = newValue;
        emit parameterChanged();
    }
}
 
// Whether to try smaller windows or not (more accurate results, but slower)
bool StereoMethodBlockMatching::getTrySmallerWindows () const
{
    return bm.state->trySmallerWindows;
}

void StereoMethodBlockMatching::setTrySmallerWindows (bool newValue)
{
    if (bm.state->trySmallerWindows != newValue) {
        bm.state->trySmallerWindows = newValue;
        emit parameterChanged();
    }
}

// Disp12MaxDiff
int StereoMethodBlockMatching::getDisp12MaxDiff () const
{
    return bm.state->disp12MaxDiff;
}

void StereoMethodBlockMatching::setDisp12MaxDiff (int newValue)
{
    if (bm.state->disp12MaxDiff != newValue) {
        bm.state->disp12MaxDiff = newValue;
        emit parameterChanged();
    }
}


//
// Depth image computation
//
void StereoMethodBlockMatching::computeDepthImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &depth)
{
    bm(img1, img2, tmpDepth);
    tmpDepth.convertTo(depth, CV_8U, 255/(bm.state->numberOfDisparities*16.));
}


void StereoMethodBlockMatching::addConfigTab (QTabWidget *tabWidget)
{
    QWidget *configTab = new QWidget(tabWidget);
    
    tabWidget->addTab(configTab, "BM");
}


