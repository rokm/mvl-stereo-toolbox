#ifndef STEREO_METHOD_BLOCK_MATCHING_GPU
#define STEREO_METHOD_BLOCK_MATCHING_GPU

#include "StereoMethod.h"

#include <opencv2/gpu/gpu.hpp>


class StereoMethodBlockMatchingGPU : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodBlockMatchingGPU (QObject * = 0);
    virtual ~StereoMethodBlockMatchingGPU ();

    // Depth image computation
    virtual void computeDepthImage (const cv::Mat &, const cv::Mat &, cv::Mat &);

    // Parameters
    int getPreset () const;
    int getNumDisparities () const;
    int getWindowSize () const;

    double getAverageTextureThreshold () const;

public slots:
    void resetToDefaults ();

    void setPreset (int);
    void setNumDisparities (int);
    void setWindowSize (int);
        
    void setAverageTextureThreshold (double);
    
protected:
    // Block matcher
    cv::gpu::StereoBM_GPU bm;
};


class ConfigTabBlockMatchingGPU : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabBlockMatchingGPU (StereoMethodBlockMatchingGPU *, QWidget * = 0);
    virtual ~ConfigTabBlockMatchingGPU ();

protected slots:
    void presetChanged (int);

    void updateParameters ();

protected:
    StereoMethodBlockMatchingGPU *method;

    QPushButton *buttonDefaults;
    QComboBox *comboBoxPreset;
    QSpinBox *spinBoxNumDisparities;
    QSpinBox *spinBoxWindowSize;
    QDoubleSpinBox *spinBoxAverageTextureThreshold;
};


#endif
