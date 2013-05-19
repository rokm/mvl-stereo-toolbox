/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
 
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

    // Disparity image computation
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &);

    // Parameter import/export
    virtual void loadParameters (const cv::FileStorage &);
    virtual void saveParameters (cv::FileStorage &) const;
    
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
