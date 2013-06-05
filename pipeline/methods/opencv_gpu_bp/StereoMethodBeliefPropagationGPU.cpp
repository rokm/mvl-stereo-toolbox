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
 
#include "StereoMethodBeliefPropagationGPU.h"
#include "StereoMethodBeliefPropagationGPUConfigWidget.h"


StereoMethodBeliefPropagationGPU::StereoMethodBeliefPropagationGPU (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "BP_GPU";
}

StereoMethodBeliefPropagationGPU::~StereoMethodBeliefPropagationGPU ()
{
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
QWidget *StereoMethodBeliefPropagationGPU::createConfigWidget (QWidget *parent)
{
    return new StereoMethodBeliefPropagationGPUConfigWidget(this, parent);
}


// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void StereoMethodBeliefPropagationGPU::usePreset (int type)
{
    switch (type) {
        case OpenCVInit: {
            // OpenCV stock
            bp = cv::gpu::StereoBeliefPropagation();
            break;
        }
        case OpenCVRecommended: {
            // OpenCV recommended parameters estimation
            bp = cv::gpu::StereoBeliefPropagation();
            bp.estimateRecommendedParams(imageWidth, imageHeight, bp.ndisp, bp.iters, bp.levels);
            break;
        }
    };

    emit parameterChanged();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void StereoMethodBeliefPropagationGPU::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    cv::gpu::GpuMat gpu_disp;

    if (1) {
        // Make sure that GPU matrices are destroyed as soon as they are
        // not needed anymore via scoping...
        cv::gpu::GpuMat gpu_img1(img1);
        cv::gpu::GpuMat gpu_img2(img2);

        // Compute disparity image
        mutex.lock();
        bp(gpu_img1, gpu_img2, gpu_disp);
        mutex.unlock();
    }
    
    // Convert and download
    cv::gpu::GpuMat gpu_disp8u;
    gpu_disp.convertTo(gpu_disp8u, CV_8U);
    gpu_disp8u.download(disparity);

    // Number of disparities
    numDisparities = getNumDisparities();
}


// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethodBeliefPropagationGPU::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
    // Load parameters
    bp = cv::gpu::StereoBeliefPropagation();
    
    storage["NumDisparities"] >> bp.ndisp;
    
    storage["Iterations"] >> bp.iters;
    storage["Levels"] >> bp.levels;
    
    storage["MaxDataTerm"] >> bp.max_data_term;
    storage["DataWeight"] >> bp.data_weight;
    storage["MaxDiscTerm"] >> bp.max_disc_term;
    storage["DiscSingleJump"] >> bp.disc_single_jump;

    emit parameterChanged();
}

void StereoMethodBeliefPropagationGPU::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);
    
    storage << "NumDisparities" << bp.ndisp;

    storage << "Iterations" << bp.iters;
    storage << "Levels" << bp.levels;
    
    storage << "MaxDataTerm" << bp.max_data_term;
    storage << "DataWeight" << bp.data_weight;
    storage << "MaxDiscTerm" << bp.max_disc_term;
    storage << "DiscSingleJump" << bp.disc_single_jump;  
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Number of disparities
int StereoMethodBeliefPropagationGPU::getNumDisparities () const
{
    return bp.ndisp;
}

void StereoMethodBeliefPropagationGPU::setNumDisparities (int newValue)
{
    setParameter(bp.ndisp, newValue);
}

// Number of iterations
int StereoMethodBeliefPropagationGPU::getIterations () const
{
    return bp.iters;
}

void StereoMethodBeliefPropagationGPU::setIterations (int newValue)
{
    setParameter(bp.iters, newValue);
}
    
// Levels
int StereoMethodBeliefPropagationGPU::getLevels () const
{
    return bp.levels;
}

void StereoMethodBeliefPropagationGPU::setLevels (int newValue)
{
    setParameter(bp.levels, newValue);
}
        
// Max data term
double StereoMethodBeliefPropagationGPU::getMaxDataTerm () const
{
    return bp.max_data_term;
}

void StereoMethodBeliefPropagationGPU::setMaxDataTerm (double newValue)
{
    setParameter(bp.max_data_term, (float)newValue);
}

// Data weight
double StereoMethodBeliefPropagationGPU::getDataWeight () const
{
    return bp.data_weight;
}

void StereoMethodBeliefPropagationGPU::setDataWeight (double newValue)
{
    setParameter(bp.data_weight, (float)newValue);
}

// Max discontinuity term
double StereoMethodBeliefPropagationGPU::getMaxDiscTerm () const
{
    return bp.max_disc_term;
}

void StereoMethodBeliefPropagationGPU::setMaxDiscTerm (double newValue)
{
    setParameter(bp.max_disc_term, (float)newValue);
}

// Single discontinuity jump
double StereoMethodBeliefPropagationGPU::getDiscSingleJump () const
{
    return bp.disc_single_jump;
}

void StereoMethodBeliefPropagationGPU::setDiscSingleJump (double newValue)
{
    setParameter(bp.disc_single_jump, (float)newValue);
}
