#ifndef STEREO_METHOD_BELIEF_PROPAGATION_GPU
#define STEREO_METHOD_BELIEF_PROPAGATION_GPU

#include "StereoMethod.h"

#include <opencv2/gpu/gpu.hpp>


class StereoMethodBeliefPropagationGPU : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodBeliefPropagationGPU (QObject * = 0);
    virtual ~StereoMethodBeliefPropagationGPU ();

    // Depth image computation
    virtual void computeDepthImage (const cv::Mat &, const cv::Mat &, cv::Mat &);

    // Parameters
    int getNumDisparities () const;

    int getIterations () const;
    int getLevels () const;

    double getMaxDataTerm () const;
    double getDataWeight () const;
    double getMaxDiscTerm () const;
    double getDiscSingleJump () const;

    enum {
        OpenCVInit,
        OpenCVRecommended,
    } PresetType;
    
public slots:
    void usePreset (int);

    void setNumDisparities (int);

    void setIterations (int);
    void setLevels (int);
        
    void setMaxDataTerm (double);
    void setDataWeight (double);
    void setMaxDiscTerm (double);
    void setDiscSingleJump (double);
    
protected:
    // Block matcher
    cv::gpu::StereoBeliefPropagation bp;
};


class ConfigTabBeliefPropagationGPU : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabBeliefPropagationGPU (StereoMethodBeliefPropagationGPU *, QWidget * = 0);
    virtual ~ConfigTabBeliefPropagationGPU ();

protected slots:
    void presetChanged (int);

    void updateParameters ();

protected:
    StereoMethodBeliefPropagationGPU *method;

    QComboBox *comboBoxPreset;
    QSpinBox *spinBoxNumDisparities;
    QSpinBox *spinBoxIterations;
    QSpinBox *spinBoxLevels;
    QDoubleSpinBox *spinBoxMaxDataTerm;
    QDoubleSpinBox *spinBoxDataWeight;
    QDoubleSpinBox *spinBoxMaxDiscTerm;
    QDoubleSpinBox *spinBoxDiscSingleJump;
};


#endif
