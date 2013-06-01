/*
 * Efficient LArge-scale Stereo: method
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

#include "StereoMethodELAS.h"
#include "StereoMethodELASConfigWidget.h"

#include <opencv2/imgproc/imgproc.hpp>


StereoMethodELAS::StereoMethodELAS (QObject *parent)
    : StereoMethod(parent), param(Elas::parameters(Elas::ROBOTICS)), elas(param), returnLeft(true)
{
    shortName = "ELAS";
    usePreset(ElasRobotics);
}

StereoMethodELAS::~StereoMethodELAS ()
{
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
QWidget *StereoMethodELAS::createConfigWidget (QWidget *parent)
{
    return new StereoMethodELASConfigWidget(this, parent);
}


// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void StereoMethodELAS::usePreset (int type)
{
    switch (type) {
        case ElasRobotics: {
            // Robotics preset
            param = Elas::parameters(Elas::ROBOTICS);
            break;
        }
        case ElasMiddlebury: {
            // Middlebury preset
            param = Elas::parameters(Elas::MIDDLEBURY);
            break;
        }
    };

    elas = Elas(param);
    emit parameterChanged();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void StereoMethodELAS::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    // Convert to grayscale
    if (img1.channels() == 3) {
        cv::cvtColor(img1, tmpImg1, CV_RGB2GRAY);
    } else {
        tmpImg1 = img1;
    }

    if (img2.channels() == 3) {
        cv::cvtColor(img2, tmpImg2, CV_RGB2GRAY);
    } else {
        tmpImg2 = img2;
    }


    // Allocate output
    int32_t dims[3] = { tmpImg1.cols, tmpImg2.rows, tmpImg1.step };

    if (param.subsampling) {
        tmpDisp1 = cv::Mat(tmpImg1.rows/2, tmpImg1.cols/2, CV_32FC1);
        tmpDisp2 = cv::Mat(tmpImg2.rows/2, tmpImg2.cols/2, CV_32FC1);
    } else {
        tmpDisp1 = cv::Mat(tmpImg1.rows, tmpImg1.cols, CV_32FC1);
        tmpDisp2 = cv::Mat(tmpImg2.rows, tmpImg2.cols, CV_32FC1);
    }

    // Process
    elas.process(tmpImg1.ptr<uint8_t>(), tmpImg2.ptr<uint8_t>(), tmpDisp1.ptr<float>(), tmpDisp2.ptr<float>(), dims);

    // Convert to output
    if (returnLeft) {
        tmpDisp1.convertTo(disparity, CV_8U);
    } else {
        tmpDisp2.convertTo(disparity, CV_8U);
    }

    // Number of disparities
    numDisparities = getMaxDisparity() - getMinDisparity();
}

// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethodELAS::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
    // Load parameters
    param = Elas::parameters(Elas::ROBOTICS);

    storage["MinDisparity"] >> param.disp_min;
    storage["MaxDisparity"] >> param.disp_max;
    
    storage["SupportThreshold"] >> param.support_threshold;
    storage["SupportTexture"] >> param.support_texture;
    storage["CandidateStepSize"] >> param.candidate_stepsize;
    storage["InconsistentWindowSize"] >> param.incon_window_size;
    storage["InconsistentThreshold"] >> param.incon_threshold;
    storage["InconsistentMinSupport"] >> param.incon_min_support;

    storage["AddCorners"] >> param.add_corners;
    storage["GridSize"] >> param.grid_size;
    
    storage["Beta"] >> param.beta;
    storage["Gamma"] >> param.gamma;
    storage["Sigma"] >> param.sigma;
    storage["SigmaRadius"] >> param.sradius;
    
    storage["MatchTexture"] >> param.match_texture;
    storage["LRThreshold"] >> param.lr_threshold;
    
    storage["SpeckleSimThreshold"] >> param.speckle_sim_threshold;
    storage["SpeckleSize"] >> param.speckle_size;
    storage["InterpolationGapWidth"] >> param.ipol_gap_width;

    storage["FilterMedian"] >> param.filter_median;
    storage["FilterAdaptiveMean"] >> param.filter_adaptive_mean;
    storage["PostProcessOnlyLeft"] >> param.postprocess_only_left;
    storage["Subsampling"] >> param.subsampling;
    
    storage["ReturnLeft"] >> returnLeft;
    
    elas = Elas(param);
    emit parameterChanged();
}

void StereoMethodELAS::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);

    // Save parameters
    storage << "MinDisparity" << param.disp_min;
    storage << "MaxDisparity" << param.disp_max;
    
    storage << "SupportThreshold" << param.support_threshold;
    storage << "SupportTexture" << param.support_texture;
    storage << "CandidateStepSize" << param.candidate_stepsize;
    storage << "InconsistentWindowSize" << param.incon_window_size;
    storage << "InconsistentThreshold" << param.incon_threshold;
    storage << "InconsistentMinSupport" << param.incon_min_support;

    storage << "AddCorners" << param.add_corners;
    storage << "GridSize" << param.grid_size;

    storage << "Beta" << param.beta;
    storage << "Gamma" << param.gamma;
    storage << "Sigma" << param.sigma;
    storage << "SigmaRadius" << param.sradius;
    
    storage << "MatchTexture" << param.match_texture;
    storage << "LRThreshold" << param.lr_threshold;
    
    storage << "SpeckleSimThreshold" << param.speckle_sim_threshold;
    storage << "SpeckleSize" << param.speckle_size;
    storage << "InterpolationGapWidth" << param.ipol_gap_width;
    
    storage << "FilterMedian" << param.filter_median;
    storage << "FilterAdaptiveMean" << param.filter_adaptive_mean;
    storage << "PostProcessOnlyLeft" << param.postprocess_only_left;
    storage << "Subsampling" << param.subsampling;

    storage << "ReturnLeft" << returnLeft;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Minimum disparity
int StereoMethodELAS::getMinDisparity () const
{
    return param.disp_min;
}

void StereoMethodELAS::setMinDisparity (int newValue)
{
    setParameter(param.disp_min, newValue);
}

// Maximum disparity
int StereoMethodELAS::getMaxDisparity () const
{
    return param.disp_max;
}

void StereoMethodELAS::setMaxDisparity (int newValue)
{
    setParameter(param.disp_max, newValue);
}


// Support threshold
double StereoMethodELAS::getSupportThreshold () const
{
    return param.support_threshold;
}

void StereoMethodELAS::setSupportThreshold (double newValue)
{
    setParameter(param.support_threshold, (float)newValue);
}

// Support texture
int StereoMethodELAS::getSupportTexture () const
{
    return param.support_texture;
}

void StereoMethodELAS::setSupportTexture (int newValue)
{
    setParameter(param.support_texture, newValue);
}

// Candidate step size
int StereoMethodELAS::getCandidateStepSize () const
{
    return param.candidate_stepsize;
}

void StereoMethodELAS::setCandidateStepSize (int newValue)
{
    setParameter(param.candidate_stepsize, newValue);
}

// Inconsistent window size
int StereoMethodELAS::getInconsistentWindowSize () const
{
    return param.incon_window_size;
}

void StereoMethodELAS::setInconsistentWindowSize (int newValue)
{
    setParameter(param.incon_window_size, newValue);
}

// Inconsistent threshold
int StereoMethodELAS::getInconsistentThreshold () const
{
    return param.incon_threshold;
}

void StereoMethodELAS::setInconsistentThreshold (int newValue)
{
    setParameter(param.incon_threshold, newValue);
}

// Inconsistent min support
int StereoMethodELAS::getInconsistentMinSupport () const
{
    return param.incon_min_support;
}

void StereoMethodELAS::setInconsistentMinSupport (int newValue)
{
    setParameter(param.incon_min_support, newValue);
}


// Add corners
bool StereoMethodELAS::getAddCorners () const
{
    return param.add_corners;
}

void StereoMethodELAS::setAddCorners (bool newValue)
{
    setParameter(param.add_corners, newValue);
}

// Grid size
int StereoMethodELAS::getGridSize () const
{
    return param.grid_size;
}

void StereoMethodELAS::setGridSize (int newValue)
{
    setParameter(param.grid_size, newValue);
}


// Beta
double StereoMethodELAS::getBeta () const
{
    return param.beta;
}

void StereoMethodELAS::setBeta (double newValue)
{
    setParameter(param.beta, (float)newValue);
}

// Gamma
double StereoMethodELAS::getGamma () const
{
    return param.gamma;
}

void StereoMethodELAS::setGamma (double newValue)
{
    setParameter(param.gamma, (float)newValue);
}

// Sigma
double StereoMethodELAS::getSigma () const
{
    return param.sigma;
}

void StereoMethodELAS::setSigma (double newValue)
{
    setParameter(param.sigma, (float)newValue);
}

// Sigma radius
double StereoMethodELAS::getSigmaRadius () const
{
    return param.sradius;
}

void StereoMethodELAS::setSigmaRadius (double newValue)
{
    setParameter(param.sradius, (float)newValue);
}


// Match texture
int StereoMethodELAS::getMatchTexture () const
{
    return param.match_texture;
}

void StereoMethodELAS::setMatchTexture (int newValue)
{
    setParameter(param.match_texture, newValue);
}

// Left/right threshold
int StereoMethodELAS::getLRThreshold () const
{
    return param.lr_threshold;
}

void StereoMethodELAS::setLRThreshold (int newValue)
{
    setParameter(param.lr_threshold, newValue);
}


// Speckle similarity threshold
double StereoMethodELAS::getSpeckleSimThreshold () const
{
    return param.speckle_sim_threshold;
}

void StereoMethodELAS::setSpeckleSimThreshold (double newValue)
{
    setParameter(param.speckle_sim_threshold, (float)newValue);
}

// Speckle size
int StereoMethodELAS::getSpeckleSize () const
{
    return param.speckle_size;
}

void StereoMethodELAS::setSpeckleSize (int newValue)
{
    setParameter(param.speckle_size, newValue);
}

// Interpolation gap width
int StereoMethodELAS::getInterpolationGapWidth () const
{
    return param.ipol_gap_width;
}

void StereoMethodELAS::setInterpolationGapWidth (int newValue)
{
    setParameter(param.ipol_gap_width, newValue);
}


// Median filter
bool StereoMethodELAS::getFilterMedian () const
{
    return param.filter_median;
}

void StereoMethodELAS::setFilterMedian (bool newValue)
{
    setParameter(param.filter_median, newValue);
}

// Adaptive mean filter
bool StereoMethodELAS::getFilterAdaptiveMean () const
{
    return param.filter_adaptive_mean;
}

void StereoMethodELAS::setFilterAdaptiveMean (bool newValue)
{
    setParameter(param.filter_adaptive_mean, newValue);
}

// Post-process only left
bool StereoMethodELAS::getPostProcessOnlyLeft () const
{
    return param.postprocess_only_left;
}

void StereoMethodELAS::setPostProcessOnlyLeft (bool newValue)
{
    setParameter(param.postprocess_only_left, newValue);
}

// Subsampling
bool StereoMethodELAS::getSubsampling () const
{
    return param.subsampling;
}

void StereoMethodELAS::setSubsampling (bool newValue)
{
    setParameter(param.subsampling, newValue);
}


// Return left/right image
bool StereoMethodELAS::getReturnLeft () const
{
    return returnLeft;
}

void StereoMethodELAS::setReturnLeft (bool newValue)
{
    setParameter(returnLeft, newValue);
}
