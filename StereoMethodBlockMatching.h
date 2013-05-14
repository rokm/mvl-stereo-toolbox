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


    int getPreFilterType () const;
    void setPreFilterType (int);
    
    int getPreFilterSize () const;
    void setPreFilterSize (int);
    
    int getPreFilterCap () const;
    void setPreFilterCap (int);
        
    
    int getSADWindowSize () const;
    void setSADWindowSize (int);
    
    int getMinDisparity () const;
    void setMinDisparity (int);

    int getNumDisparities () const;
    void setNumDisparities (int);
    
    
    int getTextureThreshold () const;
    void setTextureThreshold (int);
    
    int getUniquenessRatio () const;
    void setUniquenessRatio (int);
    
    int getSpeckleWindowSize () const;
    void setSpeckleWindowSize (int);
    
    int getSpeckleRange () const;
    void setSpeckleRange (int);
    
    bool getTrySmallerWindows () const;
    void setTrySmallerWindows (bool);
    
    int getDisp12MaxDiff () const;
    void setDisp12MaxDiff (int);
    
protected:
    // Block matcher
    cv::StereoBM bm;
    
    cv::Mat tmpDepth;
};

#endif
