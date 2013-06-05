/*
 * OpenCV GPU Block Matching: method
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
 
#include "StereoMethodBlockMatchingGPU.h"
#include "StereoMethodBlockMatchingGPUConfigWidget.h"


StereoMethodBlockMatchingGPU::StereoMethodBlockMatchingGPU (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "BM_GPU";
}

StereoMethodBlockMatchingGPU::~StereoMethodBlockMatchingGPU ()
{
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
QWidget *StereoMethodBlockMatchingGPU::createConfigWidget (QWidget *parent)
{
    return new StereoMethodBlockMatchingGPUConfigWidget(this, parent);
}


// *********************************************************************
// *                             Defaults                              *
// *********************************************************************
void StereoMethodBlockMatchingGPU::resetToDefaults ()
{
    bm = cv::gpu::StereoBM_GPU();
    emit parameterChanged();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void StereoMethodBlockMatchingGPU::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    cv::gpu::GpuMat gpu_img1, gpu_img2, gpu_disp;
    
    // Upload to GPU and convert to grayscale, if needed
    if (img1.channels() == 3) {
        cv::gpu::GpuMat gpu_tmp(img1);
        cv::gpu::cvtColor(gpu_tmp, gpu_img1, CV_RGB2GRAY);
    } else {
        gpu_img1.upload(img1);
    }

    if (img2.channels() == 3) {
        cv::gpu::GpuMat gpu_tmp(img2);
        cv::gpu::cvtColor(gpu_tmp, gpu_img2, CV_RGB2GRAY);
    } else {
        gpu_img2.upload(img2);
    }

    // Compute disparity image
    mutex.lock();
    bm(gpu_img1, gpu_img2, gpu_disp);
    mutex.unlock();

    // Download
    gpu_disp.download(disparity);

    // Number of disparities
    numDisparities = getNumDisparities();
}

// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethodBlockMatchingGPU::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
    // Load parameters
    bm = cv::gpu::StereoBM_GPU();
    
    storage["Preset"] >> bm.preset;
    storage["NumDisparities"] >> bm.ndisp;
    storage["WindowSize"] >> bm.winSize;
    storage["AverageTextureThreshold"] >> bm.avergeTexThreshold;
    
    emit parameterChanged();
}

void StereoMethodBlockMatchingGPU::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);

    // Save parameters
    storage << "Preset" << bm.preset;
    storage << "NumDisparities" << bm.ndisp;
    storage << "WindowSize" << bm.winSize;
    storage << "AverageTextureThreshold" << bm.avergeTexThreshold;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Preset
int StereoMethodBlockMatchingGPU::getPreset () const
{
    return bm.preset;
}

void StereoMethodBlockMatchingGPU::setPreset (int newValue)
{
    // Validate
    if (newValue != cv::gpu::StereoBM_GPU::BASIC_PRESET && newValue != cv::gpu::StereoBM_GPU::PREFILTER_XSOBEL) {
        newValue = cv::gpu::StereoBM_GPU::BASIC_PRESET;
    }

    setParameter(bm.preset, newValue);
}

// Number of disparities
int StereoMethodBlockMatchingGPU::getNumDisparities () const
{
    return bm.ndisp;
}

void StereoMethodBlockMatchingGPU::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 8.0) * 8; // Must be divisible by 8
    newValue = qMax(8, newValue);

    setParameter(bm.ndisp, newValue);
}
    
// Window size
int StereoMethodBlockMatchingGPU::getWindowSize () const
{
    return bm.winSize;
}

void StereoMethodBlockMatchingGPU::setWindowSize (int newValue)
{
    setParameter(bm.winSize, newValue);
}
        

// Average texture threshold
double StereoMethodBlockMatchingGPU::getAverageTextureThreshold () const
{
    return bm.avergeTexThreshold;
}

void StereoMethodBlockMatchingGPU::setAverageTextureThreshold (double newValue)
{
    setParameter(bm.avergeTexThreshold, (float)newValue);
}
