#ifndef STEREO_METHOD_BLOCK_MATCHING
#define STEREO_METHOD_BLOCK_MATCHING

#include "StereoMethod.h"

#include <opencv2/calib3d/calib3d.hpp>


class StereoMethodBlockMatching : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodBlockMatching (QObject * = 0);
    virtual ~StereoMethodBlockMatching ();

    virtual void computeDepthImage (const cv::Mat &, const cv::Mat &, cv::Mat &);
    virtual void addConfigTab (QTabWidget *);

    int getPreFilterType () const;
    int getPreFilterSize () const;
    int getPreFilterCap () const;

    int getSADWindowSize () const;
    int getMinDisparity () const;
    int getNumDisparities () const;

    int getTextureThreshold () const;
    int getUniquenessRatio () const;
    int getSpeckleWindowSize () const;
    int getSpeckleRange () const;

    bool getTrySmallerWindows () const;

    int getDisp12MaxDiff () const;

public slots:
    void setPreFilterType (int);
    void setPreFilterSize (int);
    void setPreFilterCap (int);
        
    void setSADWindowSize (int);
    void setMinDisparity (int);
    void setNumDisparities (int);
    
    
    void setTextureThreshold (int);
    void setUniquenessRatio (int);
    void setSpeckleWindowSize (int);
    void setSpeckleRange (int);
    
    void setTrySmallerWindows (bool);
    
    void setDisp12MaxDiff (int);
    
protected:
    // Block matcher
    cv::StereoBM bm;
    
    cv::Mat tmpDepth;
};


class ConfigTabBlockMatching : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabBlockMatching (StereoMethodBlockMatching *, QWidget * = 0);
    virtual ~ConfigTabBlockMatching ();

protected slots:
    void currentIndexChanged (int);
    void trySmallerWindowsChanged (int);

    void updateParameters ();

protected:
    StereoMethodBlockMatching *method;

    QComboBox *comboBoxPreFilterType;
    QSpinBox *spinBoxPreFilterSize;
    QSpinBox *spinBoxPreFilterCap;
    QSpinBox *spinBoxSADWindowSize;
    QSpinBox *spinBoxMinDisparity;
    QSpinBox *spinBoxNumDisparities;
    QSpinBox *spinBoxTextureThreshold;
    QSpinBox *spinBoxUniquenessRatio;
    QSpinBox *spinBoxSpeckleWindowSize;
    QSpinBox *spinBoxSpeckleRange;
    QCheckBox *checkBoxTrySmallerWindow;
    QSpinBox *spinBoxDisp12MaxDiff;
};


#endif
