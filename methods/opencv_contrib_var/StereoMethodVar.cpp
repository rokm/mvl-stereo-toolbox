/*
 * OpenCV Variational Matching: method
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
 
#include "StereoMethodVar.h"
#include "StereoMethodVarConfigWidget.h"

#include <opencv2/imgproc/imgproc.hpp>


StereoMethodVar::StereoMethodVar (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "Var";

    usePreset(OpenCV);
}

StereoMethodVar::~StereoMethodVar ()
{
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
QWidget *StereoMethodVar::createConfigWidget (QWidget *parent)
{
    return new StereoMethodVarConfigWidget(this, parent);
}


// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void StereoMethodVar::usePreset (int type)
{
    switch (type) {
        case OpenCV: {
            // OpenCV
            var = cv::StereoVar();
            //var.numberOfDisparities = ((imageWidth/8) + 15) & -16;
            break;
        }
        case StereoMatch: {
            // "Stereo match" example
            var = cv::StereoVar();

            var.levels = 3;
            var.pyrScale = 0.5;
            var.nIt = 25;
            var.minDisp = -(((imageWidth/8) + 15) & -16);
            var.maxDisp = 0;
            var.poly_n = 3;
            var.poly_sigma = 0.0;
            var.fi = 15.0f;
            var.lambda = 0.03f;
            var.penalization = var.PENALIZATION_TICHONOV;
            var.cycle = var.CYCLE_V;
            var.flags = var.USE_SMART_ID | var.USE_AUTO_PARAMS | var.USE_INITIAL_DISPARITY | var.USE_MEDIAN_FILTERING;

            break;
        }
    };

    emit parameterChanged();
}



// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void StereoMethodVar::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{    
    // Compute disparity image
    var(img1, img2, tmpDisparity);

    // Normalize to output
    tmpDisparity.convertTo(disparity, CV_8U);

    // Number of disparities
    numDisparities = getMaxDisparity() - getMinDisparity();
}


// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethodVar::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
    // Load parameters
    var = cv::StereoVar();
    
    storage["Levels"] >> var.levels;
    storage["PyrScale"] >> var.pyrScale;
    storage["NumIterations"] >> var.nIt;
    storage["MinDisparity"] >> var.minDisp;
    storage["MaxDisparity"] >> var.maxDisp;
    storage["PolyN"] >> var.poly_n;
    storage["PolySigma"] >> var.poly_sigma;
    storage["Fi"] >> var.fi;
    storage["Lambda"] >> var.lambda;
    storage["Penalization"] >> var.penalization;
    storage["Cycle"] >> var.cycle;
    storage["Flags"] >> var.flags;
    
    emit parameterChanged();
}

void StereoMethodVar::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);

    // Save parameters
    storage << "Levels" << var.levels;
    storage << "PyrScale" << var.pyrScale;
    storage << "NumIterations" << var.nIt;
    storage << "MinDisparity" << var.minDisp;
    storage << "MaxDisparity" << var.maxDisp;
    storage << "PolyN" << var.poly_n;
    storage << "PolySigma" << var.poly_sigma;
    storage << "Fi" << var.fi;
    storage << "Lambda" << var.lambda;
    storage << "Penalization" << var.penalization;
    storage << "Cycle" << var.cycle;
    storage << "Flags" << var.flags;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Levels
int StereoMethodVar::getLevels () const
{
    return var.levels;
}

void StereoMethodVar::setLevels (int newValue)
{
    setParameter(var.levels, newValue);
}

// Pyramid scale
double StereoMethodVar::getPyrScale () const
{
    return var.pyrScale;
}

void StereoMethodVar::setPyrScale (double newValue)
{
    setParameter(var.pyrScale, newValue);
}
    
// Number of iterations
int StereoMethodVar::getNumIterations () const
{
    return var.nIt;
}

void StereoMethodVar::setNumIterations (int newValue)
{
    setParameter(var.nIt, newValue);
}
        

// Minimum disparity
int StereoMethodVar::getMinDisparity () const
{
    return var.minDisp;
}

void StereoMethodVar::setMinDisparity (int newValue)
{
    setParameter(var.minDisp, newValue);
}
    
// Maximum disparity
int StereoMethodVar::getMaxDisparity () const
{
    return var.maxDisp;
}

void StereoMethodVar::setMaxDisparity (int newValue)
{
    setParameter(var.maxDisp, newValue);
}

// PolyN
int StereoMethodVar::getPolyN () const
{
    return var.poly_n;
}

void StereoMethodVar::setPolyN (int newValue)
{
    setParameter(var.poly_n, newValue);
}

// PolySigma
double StereoMethodVar::getPolySigma () const
{
    return var.poly_sigma;
}

void StereoMethodVar::setPolySigma (double newValue)
{
    setParameter(var.poly_sigma, newValue);
}

// Fi
double StereoMethodVar::getFi () const
{
    return var.fi;
}

void StereoMethodVar::setFi (double newValue)
{
    setParameter(var.fi, (float)newValue);
}

// Lambda
double StereoMethodVar::getLambda () const
{
    return var.lambda;
}

void StereoMethodVar::setLambda (double newValue)
{
    setParameter(var.lambda, (float)newValue);
}
 
// Penalization
int StereoMethodVar::getPenalization () const
{
    return var.penalization;
}

void StereoMethodVar::setPenalization (int newValue)
{
    if (newValue != cv::StereoVar::PENALIZATION_TICHONOV &&
        newValue != cv::StereoVar::PENALIZATION_CHARBONNIER &&
        newValue != cv::StereoVar::PENALIZATION_PERONA_MALIK) {
        newValue = cv::StereoVar::PENALIZATION_TICHONOV;
    }

    setParameter(var.penalization, newValue);
}

// Cycle
int StereoMethodVar::getCycle () const
{
    return var.cycle;
}

void StereoMethodVar::setCycle (int newValue)
{
    if (newValue != cv::StereoVar::CYCLE_O &&
        newValue != cv::StereoVar::CYCLE_V) {
        newValue = cv::StereoVar::CYCLE_O;
    }

    setParameter(var.cycle, newValue);
}

// Flags
int StereoMethodVar::getFlags () const
{
    return var.flags;
}

void StereoMethodVar::setFlags (int newValue)
{
    setParameter(var.flags, newValue);
}
