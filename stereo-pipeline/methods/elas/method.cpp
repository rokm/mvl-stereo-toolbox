/*
 * Efficient LArge-scale Stereo: method
 * Copyright (C) 2013-2017 Rok Mandeljc
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

#include <opencv2/imgproc.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodELAS {


Method::Method (QObject *parent)
    : QObject(parent), StereoMethod(),
      param(Elas::parameters(Elas::ROBOTICS)),
      elas(param), returnLeft(true)
{
    usePreset(ElasRobotics);
}

Method::~Method ()
{
}


void Method::createElasObject ()
{
    QMutexLocker locker(&mutex);
    elas = Elas(param);
    locker.unlock();

    emit parameterChanged();
}


// *********************************************************************
// *                       StereoMethod interface                      *
// *********************************************************************
QString Method::getShortName () const
{
    return "ELAS";
}

QWidget *Method::createConfigWidget (QWidget *parent)
{
    return new MethodWidget(this, parent);
}


// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void Method::usePreset (int preset)
{
    switch (preset) {
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

    // Create ELAS object
    createElasObject();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void Method::computeDisparity (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    // Convert to grayscale
    if (img1.channels() == 3) {
        cv::cvtColor(img1, tmpImg1, cv::COLOR_BGR2GRAY);
    } else {
        tmpImg1 = img1;
    }

    if (img2.channels() == 3) {
        cv::cvtColor(img2, tmpImg2, cv::COLOR_BGR2GRAY);
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
    QMutexLocker locker(&mutex);
    elas.process(tmpImg1.ptr<uint8_t>(), tmpImg2.ptr<uint8_t>(), tmpDisp1.ptr<float>(), tmpDisp2.ptr<float>(), dims);
    locker.unlock();

    // Copy corresponding output (already in float format)
    if (returnLeft) {
        tmpDisp1.copyTo(disparity);
    } else {
        tmpDisp2.copyTo(disparity);
    }

    // Number of disparities
    numDisparities = getMaxDisparity() - getMinDisparity();
}

// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void Method::loadParameters (const QString &filename)
{
    // Open storage
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::READ);
    if (!storage.isOpened()) {
        throw QString("Cannot open file \"%1\" for reading!").arg(filename);
    }

    // Validate data type
    QString dataType = QString::fromStdString(storage["DataType"]);
    if (dataType.compare("StereoMethodParameters")) {
        throw QString("Invalid stereo method parameters configuration!");
    }

    // Validate method name
    QString storedName = QString::fromStdString(storage["MethodName"]);
    if (storedName.compare(getShortName())) {
        throw QString("Invalid configuration for method \"%1\"!").arg(getShortName());
    }

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

    // Create ELAS object
    createElasObject();
}

void Method::saveParameters (const QString &filename) const
{
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::WRITE);
    if (!storage.isOpened()) {
        throw QString("Cannot open file \"%1\" for writing!").arg(filename);
    }

    // Data type
    storage << "DataType" << "StereoMethodParameters";

    // Store method name, so it can be validate upon loading
    storage << "MethodName" << getShortName().toStdString();

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
int Method::getMinDisparity () const
{
    return param.disp_min;
}

void Method::setMinDisparity (int value)
{
    setParameter(param.disp_min, value);
}

// Maximum disparity
int Method::getMaxDisparity () const
{
    return param.disp_max;
}

void Method::setMaxDisparity (int value)
{
    setParameter(param.disp_max, value);
}


// Support threshold
double Method::getSupportThreshold () const
{
    return param.support_threshold;
}

void Method::setSupportThreshold (double value)
{
    setParameter(param.support_threshold, (float)value);
}

// Support texture
int Method::getSupportTexture () const
{
    return param.support_texture;
}

void Method::setSupportTexture (int value)
{
    setParameter(param.support_texture, value);
}

// Candidate step size
int Method::getCandidateStepSize () const
{
    return param.candidate_stepsize;
}

void Method::setCandidateStepSize (int value)
{
    setParameter(param.candidate_stepsize, value);
}

// Inconsistent window size
int Method::getInconsistentWindowSize () const
{
    return param.incon_window_size;
}

void Method::setInconsistentWindowSize (int value)
{
    setParameter(param.incon_window_size, value);
}

// Inconsistent threshold
int Method::getInconsistentThreshold () const
{
    return param.incon_threshold;
}

void Method::setInconsistentThreshold (int value)
{
    setParameter(param.incon_threshold, value);
}

// Inconsistent min support
int Method::getInconsistentMinSupport () const
{
    return param.incon_min_support;
}

void Method::setInconsistentMinSupport (int value)
{
    setParameter(param.incon_min_support, value);
}


// Add corners
bool Method::getAddCorners () const
{
    return param.add_corners;
}

void Method::setAddCorners (bool value)
{
    setParameter(param.add_corners, value);
}

// Grid size
int Method::getGridSize () const
{
    return param.grid_size;
}

void Method::setGridSize (int value)
{
    setParameter(param.grid_size, value);
}


// Beta
double Method::getBeta () const
{
    return param.beta;
}

void Method::setBeta (double value)
{
    setParameter(param.beta, (float)value);
}

// Gamma
double Method::getGamma () const
{
    return param.gamma;
}

void Method::setGamma (double value)
{
    setParameter(param.gamma, (float)value);
}

// Sigma
double Method::getSigma () const
{
    return param.sigma;
}

void Method::setSigma (double value)
{
    setParameter(param.sigma, (float)value);
}

// Sigma radius
double Method::getSigmaRadius () const
{
    return param.sradius;
}

void Method::setSigmaRadius (double value)
{
    setParameter(param.sradius, (float)value);
}


// Match texture
int Method::getMatchTexture () const
{
    return param.match_texture;
}

void Method::setMatchTexture (int value)
{
    setParameter(param.match_texture, value);
}

// Left/right threshold
int Method::getLRThreshold () const
{
    return param.lr_threshold;
}

void Method::setLRThreshold (int value)
{
    setParameter(param.lr_threshold, value);
}


// Speckle similarity threshold
double Method::getSpeckleSimThreshold () const
{
    return param.speckle_sim_threshold;
}

void Method::setSpeckleSimThreshold (double value)
{
    setParameter(param.speckle_sim_threshold, (float)value);
}

// Speckle size
int Method::getSpeckleSize () const
{
    return param.speckle_size;
}

void Method::setSpeckleSize (int value)
{
    setParameter(param.speckle_size, value);
}

// Interpolation gap width
int Method::getInterpolationGapWidth () const
{
    return param.ipol_gap_width;
}

void Method::setInterpolationGapWidth (int value)
{
    setParameter(param.ipol_gap_width, value);
}


// Median filter
bool Method::getFilterMedian () const
{
    return param.filter_median;
}

void Method::setFilterMedian (bool value)
{
    setParameter(param.filter_median, value);
}

// Adaptive mean filter
bool Method::getFilterAdaptiveMean () const
{
    return param.filter_adaptive_mean;
}

void Method::setFilterAdaptiveMean (bool value)
{
    setParameter(param.filter_adaptive_mean, value);
}

// Post-process only left
bool Method::getPostProcessOnlyLeft () const
{
    return param.postprocess_only_left;
}

void Method::setPostProcessOnlyLeft (bool value)
{
    setParameter(param.postprocess_only_left, value);
}

// Subsampling
bool Method::getSubsampling () const
{
    return param.subsampling;
}

void Method::setSubsampling (bool value)
{
    setParameter(param.subsampling, value);
}


// Return left/right image
bool Method::getReturnLeft () const
{
    return returnLeft;
}

void Method::setReturnLeft (bool value)
{
    setParameter(returnLeft, value);
}


} // StereoMethodELAS
} // Pipeline
} // StereoToolbox
} // MVL
