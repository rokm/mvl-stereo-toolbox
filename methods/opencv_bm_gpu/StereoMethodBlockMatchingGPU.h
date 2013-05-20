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

    // Disparity image computation
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &, int &);

    // Parameter import/export
    virtual void loadParameters (const cv::FileStorage &);
    virtual void saveParameters (cv::FileStorage &) const;
    
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
