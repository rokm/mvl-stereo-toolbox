#ifndef STEREO_METHOD_BLOCK_MATCHING
#define STEREO_METHOD_BLOCK_MATCHING

#include <QtCore>
#include <opencv2/calib3d/calib3d.hpp>

#include "StereoMethod.h"

class StereoMethodBlockMatching : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodBlockMatching (QObject * = 0);
    virtual ~StereoMethodBlockMatching ();

    virtual void computeDepthImage (const cv::Mat &, const cv::Mat &, cv::Mat &);


protected:

    // Parameters
    int preset;
    int numDisparities;
    int windowSize;

    // Block matcher
    cv::StereoBM bm;
};

#endif
