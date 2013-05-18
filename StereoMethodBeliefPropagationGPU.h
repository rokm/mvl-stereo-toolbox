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

    // Parameter import/export
    virtual void loadParameters (const cv::FileStorage &);
    virtual void saveParameters (cv::FileStorage &) const;

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
