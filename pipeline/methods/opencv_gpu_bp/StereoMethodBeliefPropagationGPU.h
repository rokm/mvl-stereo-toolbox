/*
 * OpenCV GPU Belief Propagation: method
 * Copyright (C) 2013 Rok Mandeljc
 * 
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
 
#ifndef STEREO_METHOD_BELIEF_PROPAGATION_GPU_H
#define STEREO_METHOD_BELIEF_PROPAGATION_GPU_H

#include "StereoMethod.h"

#include <opencv2/gpu/gpu.hpp>


class StereoMethodBeliefPropagationGPU : public StereoMethod
{
    Q_OBJECT

public:
    StereoMethodBeliefPropagationGPU (QObject * = 0);
    virtual ~StereoMethodBeliefPropagationGPU ();

    // Config widget
    virtual QWidget *createConfigWidget (QWidget * = 0);
    
    // Disparity image computation
    virtual void computeDisparityImage (const cv::Mat &, const cv::Mat &, cv::Mat &, int &);

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

#endif