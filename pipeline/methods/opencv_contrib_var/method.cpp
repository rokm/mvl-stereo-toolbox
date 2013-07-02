/*
 * OpenCV Variational Matching: method
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#include "method.h"
#include "method_widget.h"

#include <opencv2/imgproc/imgproc.hpp>

using namespace StereoMethodVar;


Method::Method (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "Var";
    usePreset(OpenCV);
}

Method::~Method ()
{
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
QWidget *Method::createConfigWidget (QWidget *parent)
{
    return new MethodWidget(this, parent);
}


// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void Method::usePreset (int type)
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
void Method::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{    
    // Compute disparity image
    mutex.lock();
    var(img1, img2, tmpDisparity);
    mutex.unlock();

    // Normalize to output
    tmpDisparity.convertTo(disparity, CV_8U);

    // Number of disparities
    numDisparities = getMaxDisparity() - getMinDisparity();
}


// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void Method::loadParameters (const cv::FileStorage &storage)
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

void Method::saveParameters (cv::FileStorage &storage) const
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
int Method::getLevels () const
{
    return var.levels;
}

void Method::setLevels (int newValue)
{
    setParameter(var.levels, newValue);
}

// Pyramid scale
double Method::getPyrScale () const
{
    return var.pyrScale;
}

void Method::setPyrScale (double newValue)
{
    setParameter(var.pyrScale, newValue);
}
    
// Number of iterations
int Method::getNumIterations () const
{
    return var.nIt;
}

void Method::setNumIterations (int newValue)
{
    setParameter(var.nIt, newValue);
}
        

// Minimum disparity
int Method::getMinDisparity () const
{
    return var.minDisp;
}

void Method::setMinDisparity (int newValue)
{
    setParameter(var.minDisp, newValue);
}
    
// Maximum disparity
int Method::getMaxDisparity () const
{
    return var.maxDisp;
}

void Method::setMaxDisparity (int newValue)
{
    setParameter(var.maxDisp, newValue);
}

// PolyN
int Method::getPolyN () const
{
    return var.poly_n;
}

void Method::setPolyN (int newValue)
{
    setParameter(var.poly_n, newValue);
}

// PolySigma
double Method::getPolySigma () const
{
    return var.poly_sigma;
}

void Method::setPolySigma (double newValue)
{
    setParameter(var.poly_sigma, newValue);
}

// Fi
double Method::getFi () const
{
    return var.fi;
}

void Method::setFi (double newValue)
{
    setParameter(var.fi, (float)newValue);
}

// Lambda
double Method::getLambda () const
{
    return var.lambda;
}

void Method::setLambda (double newValue)
{
    setParameter(var.lambda, (float)newValue);
}
 
// Penalization
int Method::getPenalization () const
{
    return var.penalization;
}

void Method::setPenalization (int newValue)
{
    if (newValue != cv::StereoVar::PENALIZATION_TICHONOV &&
        newValue != cv::StereoVar::PENALIZATION_CHARBONNIER &&
        newValue != cv::StereoVar::PENALIZATION_PERONA_MALIK) {
        newValue = cv::StereoVar::PENALIZATION_TICHONOV;
    }

    setParameter(var.penalization, newValue);
}

// Cycle
int Method::getCycle () const
{
    return var.cycle;
}

void Method::setCycle (int newValue)
{
    if (newValue != cv::StereoVar::CYCLE_O &&
        newValue != cv::StereoVar::CYCLE_V) {
        newValue = cv::StereoVar::CYCLE_O;
    }

    setParameter(var.cycle, newValue);
}

// Flags
int Method::getFlags () const
{
    return var.flags;
}

void Method::setFlags (int newValue)
{
    setParameter(var.flags, newValue);
}
