#include "StereoMethodBlockMatching.h"


StereoMethodBlockMatching::StereoMethodBlockMatching (QObject *parent)
    : StereoMethod(parent)
{

}

StereoMethodBlockMatching::~StereoMethodBlockMatching ()
{
}


//
// Parameters
//


void StereoMethodBlockMatching::computeDepthImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &depth)
{
    bm(img1, img2, depth);
}


