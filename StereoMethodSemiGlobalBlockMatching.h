#ifndef STEREO_METHOD_SEMI_GLOBAL_BLOCK_MATCHING
#define STEREO_METHOD_SEMI_GLOBAL_BLOCK_MATCHING

#include <QtCore>
#include <opencv2/calib3d/calib3d.hpp>

#include "StereoMethod.h"

class StereoMethodSemiGlobalBlockMatching : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodSemiGlobalBlockMatching (QObject * = 0);
    virtual ~StereoMethodSemiGlobalBlockMatching ();

    virtual void computeDepthImage (const cv::Mat &, const cv::Mat &, cv::Mat &);


    int getMinDisparity () const;
    void setMinDisparity (int);
    
    int getNumDisparities () const;
    void setNumDisparities (int);
    
    int getSADWindowSize () const;
    void setSADWindowSize (int);
    
    int getPreFilterCap () const;
    void setPreFilterCap (int);
    
    int getUniquenessRatio () const;
    void setUniquenessRatio (int);
    
    int getP1 () const;
    void setP1 (int);
    
    int getP2 () const;
    void setP2 (int);
    
    int getSpeckleWindowSize () const;
    void setSpeckleWindowSize (int);
    
    int getSpeckleRange () const;
    void setSpeckleRange (int);
    
    int getDisp12MaxDiff () const;
    void setDisp12MaxDiff (int);
    
    bool getFullDP () const;
    void setFullDP (bool);
    
protected:
    // Semi-global block matcher
    cv::StereoSGBM sgbm;
};

#endif
