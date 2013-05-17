#ifndef STEREO_METHOD_CONSTANT_SPACE_BELIEF_PROPAGATION_GPU
#define STEREO_METHOD_CONSTANT_SPACE_BELIEF_PROPAGATION_GPU

#include "StereoMethod.h"

#include <opencv2/gpu/gpu.hpp>


class StereoMethodConstantSpaceBeliefPropagationGPU : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodConstantSpaceBeliefPropagationGPU (QObject * = 0);
    virtual ~StereoMethodConstantSpaceBeliefPropagationGPU ();

    // Depth image computation
    virtual void computeDepthImage (const cv::Mat &, const cv::Mat &, cv::Mat &);

    // Parameters
    int getNumDisparities () const;

    int getIterations () const;
    int getLevels () const;
    int getNrPlane () const;

    double getMaxDataTerm () const;
    double getDataWeight () const;
    double getMaxDiscTerm () const;
    double getDiscSingleJump () const;
    int getMinDispThreshold () const;

    bool getUseLocalCost () const;

    enum {
        OpenCVInit,
        OpenCVRecommended,
    } PresetType;
    
public slots:
    void usePreset (int);

    void setNumDisparities (int);

    void setIterations (int);
    void setLevels (int);
    void setNrPlane (int);
        
    void setMaxDataTerm (double);
    void setDataWeight (double);
    void setMaxDiscTerm (double);
    void setDiscSingleJump (double);
    void setMinDispThreshold (int);

    void setUseLocalCost (bool);
    
protected:
    // Block matcher
    cv::gpu::StereoConstantSpaceBP bp;
};


class ConfigTabConstantSpaceBeliefPropagationGPU : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabConstantSpaceBeliefPropagationGPU (StereoMethodConstantSpaceBeliefPropagationGPU *, QWidget * = 0);
    virtual ~ConfigTabConstantSpaceBeliefPropagationGPU ();

protected slots:
    void presetChanged (int);

    void localCostChanged (int);

    void updateParameters ();

protected:
    StereoMethodConstantSpaceBeliefPropagationGPU *method;

    QComboBox *comboBoxPreset;
    QSpinBox *spinBoxNumDisparities;
    QSpinBox *spinBoxIterations;
    QSpinBox *spinBoxLevels;
    QSpinBox *spinBoxNrPlane;
    QDoubleSpinBox *spinBoxMaxDataTerm;
    QDoubleSpinBox *spinBoxDataWeight;
    QDoubleSpinBox *spinBoxMaxDiscTerm;
    QDoubleSpinBox *spinBoxDiscSingleJump;
    QSpinBox *spinBoxMinDispThreshold;
    QCheckBox *checkBoxUseLocalCost;
};


#endif
