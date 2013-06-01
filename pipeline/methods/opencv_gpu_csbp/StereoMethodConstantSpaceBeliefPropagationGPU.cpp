/*
 * OpenCV GPU Constant Space Belief Propagation: method
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
 
#include "StereoMethodConstantSpaceBeliefPropagationGPU.h"
#include "StereoMethodConstantSpaceBeliefPropagationGPUConfigWidget.h"


StereoMethodConstantSpaceBeliefPropagationGPU::StereoMethodConstantSpaceBeliefPropagationGPU (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "CSBP_GPU";
    usePreset(OpenCVInit);
}

StereoMethodConstantSpaceBeliefPropagationGPU::~StereoMethodConstantSpaceBeliefPropagationGPU ()
{
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
QWidget *StereoMethodConstantSpaceBeliefPropagationGPU::createConfigWidget (QWidget *parent)
{
    return new StereoMethodConstantSpaceBeliefPropagationGPUConfigWidget(this, parent);
}


// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void StereoMethodConstantSpaceBeliefPropagationGPU::usePreset (int type)
{
    switch (type) {
        case OpenCVInit: {
            // OpenCV stock
            bp = cv::gpu::StereoConstantSpaceBP();
            break;
        }
        case OpenCVRecommended: {
            // OpenCV recommended parameters estimation
            bp = cv::gpu::StereoConstantSpaceBP();
            bp.estimateRecommendedParams(imageWidth, imageHeight, bp.ndisp, bp.iters, bp.levels, bp.nr_plane);
            break;
        }
    };

    emit parameterChanged();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void StereoMethodConstantSpaceBeliefPropagationGPU::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    cv::gpu::GpuMat gpu_disp;

    if (1) {
        // Make sure that GPU matrices are destroyed as soon as they are
        // not needed anymore via scoping...
        cv::gpu::GpuMat gpu_img1(img1);
        cv::gpu::GpuMat gpu_img2(img2);

        // Compute disparity image
        bp(gpu_img1, gpu_img2, gpu_disp);
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
void StereoMethodConstantSpaceBeliefPropagationGPU::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
    // Load parameters
    bp = cv::gpu::StereoConstantSpaceBP();

    storage["NumDisparities"] >> bp.ndisp;
    
    storage["Iterations"] >> bp.iters;
    storage["Levels"] >> bp.levels;
    storage["NrPlane"] >> bp.nr_plane;

    storage["MaxDataTerm"] >> bp.max_data_term;
    storage["DataWeight"] >> bp.data_weight;
    storage["MaxDiscTerm"] >> bp.max_disc_term;
    storage["DiscSingleJump"] >> bp.disc_single_jump;
    storage["MinDispThreshold"] >> bp.min_disp_th;

    storage["UseLocalCost"] >> bp.use_local_init_data_cost;
    
    emit parameterChanged();
}

void StereoMethodConstantSpaceBeliefPropagationGPU::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);

    // Save parameters
    storage << "NumDisparities" << bp.ndisp;
    
    storage << "Iterations" << bp.iters;
    storage << "Levels" << bp.levels;
    storage << "NrPlane" << bp.nr_plane;

    storage << "MaxDataTerm" << bp.max_data_term;
    storage << "DataWeight" << bp.data_weight;
    storage << "MaxDiscTerm" << bp.max_disc_term;
    storage << "DiscSingleJump" << bp.disc_single_jump;
    storage << "MinDispThreshold" << bp.min_disp_th;

    storage << "UseLocalCost" << bp.use_local_init_data_cost;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Number of disparities
int StereoMethodConstantSpaceBeliefPropagationGPU::getNumDisparities () const
{
    return bp.ndisp;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setNumDisparities (int newValue)
{
    setParameter(bp.ndisp, newValue);
}

// Number of iterations
int StereoMethodConstantSpaceBeliefPropagationGPU::getIterations () const
{
    return bp.iters;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setIterations (int newValue)
{
    setParameter(bp.iters, newValue);
}
   
// Levels
int StereoMethodConstantSpaceBeliefPropagationGPU::getLevels () const
{
    return bp.levels;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setLevels (int newValue)
{
    setParameter(bp.levels, newValue);
}

// Number of disparity levels on first level
int StereoMethodConstantSpaceBeliefPropagationGPU::getNrPlane () const
{
    return bp.nr_plane;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setNrPlane (int newValue)
{
    setParameter(bp.nr_plane, newValue);
}

// Max data term
double StereoMethodConstantSpaceBeliefPropagationGPU::getMaxDataTerm () const
{
    return bp.max_data_term;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setMaxDataTerm (double newValue)
{
    setParameter(bp.max_data_term, (float)newValue);
}

// Data weight
double StereoMethodConstantSpaceBeliefPropagationGPU::getDataWeight () const
{
    return bp.data_weight;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setDataWeight (double newValue)
{
    setParameter(bp.data_weight, (float)newValue);
}

// Max discontinuity term
double StereoMethodConstantSpaceBeliefPropagationGPU::getMaxDiscTerm () const
{
    return bp.max_disc_term;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setMaxDiscTerm (double newValue)
{
    setParameter(bp.max_disc_term, (float)newValue);
}

// Single discontinuity jump
double StereoMethodConstantSpaceBeliefPropagationGPU::getDiscSingleJump () const
{
    return bp.disc_single_jump;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setDiscSingleJump (double newValue)
{
    setParameter(bp.disc_single_jump, (float)newValue);
}

// Minimal disparity threshold
int StereoMethodConstantSpaceBeliefPropagationGPU::getMinDispThreshold () const
{
    return bp.min_disp_th;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setMinDispThreshold (int newValue)
{
    setParameter(bp.min_disp_th, newValue);
}

// Use local cost
bool StereoMethodConstantSpaceBeliefPropagationGPU::getUseLocalCost () const
{
    return bp.use_local_init_data_cost;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setUseLocalCost (bool newValue)
{
    setParameter(bp.use_local_init_data_cost, newValue);
}
