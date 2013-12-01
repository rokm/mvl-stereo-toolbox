/*
 * OpenCV GPU Dense Optical Flow - Dual TVL1: method
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

using namespace StereoMethodFlowDualTVL1Gpu;


Method::Method (QObject *parent)
    : QObject(parent), StereoMethod(), reverseImages(false)
{
}

Method::~Method ()
{
}


// *********************************************************************
// *                       StereoMethod interface                      *
// *********************************************************************
QString Method::getShortName () const
{
    return "Flow_DualTVL1_GPU";
}

QWidget *Method::createConfigWidget (QWidget *parent)
{
    return new MethodWidget(this, parent);
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void Method::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    cv::gpu::GpuMat gpu_img1, gpu_img2, gpu_disp_u, gpu_disp_v;

    // Convert to float grayscale
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

    // Upload to GPU
    gpu_img1.upload(tmpImg1);
    gpu_img2.upload(tmpImg2);
    
    // Compute flow
    QMutexLocker locker(&mutex);
    if (reverseImages) {
        flow(gpu_img1, gpu_img2, gpu_disp_u, gpu_disp_v);
    } else {
        flow(gpu_img2, gpu_img1, gpu_disp_u, gpu_disp_v);
    }
    locker.unlock();

    // Download u-disparity
    gpu_disp_u.download(tmpDisp);

    // Find max
    double max;
    cv::minMaxLoc(tmpDisp, NULL, &max);

    // Convert and return
    tmpDisp.convertTo(disparity, CV_8U);
    numDisparities = max;
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
    storage["ReverseImages"] >> reverseImages;
    storage["Tau"] >> flow.tau;
    storage["Lambda"] >> flow.lambda;
    storage["Theta"] >> flow.theta;
    storage["NumberOfScales"] >> flow.nscales;
    storage["NumberOfWarps"] >> flow.warps;
    storage["Epsilon"] >> flow.epsilon;
    storage["NumberOfIterations"] >> flow.iterations;
    
    emit parameterChanged();
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
    storage << "ReverseImages" <<  reverseImages;
    storage << "Tau" <<  flow.tau;
    storage << "Lambda" <<  flow.lambda;
    storage << "Theta" <<  flow.theta;
    storage << "NumberOfScales" <<  flow.nscales;
    storage << "NumberOfWarps" <<  flow.warps;
    storage << "Epsilon" <<  flow.epsilon;
    storage << "NumberOfIterations" <<  flow.iterations;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Reverse images
bool Method::getReverseImages () const
{
    return reverseImages;
}

void Method::setReverseImages (bool newValue)
{
    setParameter<bool>(reverseImages, newValue);
}

// Tau
double Method::getTau () const
{
    return flow.tau;
}

void Method::setTau (double newValue)
{
    setParameter<double>(flow.tau, newValue);
}

// Lambda
double Method::getLambda () const
{
    return flow.lambda;
}

void Method::setLambda (double newValue)
{
    setParameter<double>(flow.lambda, newValue);
}

// Theta
double Method::getTheta () const
{
    return flow.theta;
}

void Method::setTheta (double newValue)
{
    setParameter<double>(flow.theta, newValue);
}

// Number of scales
int Method::getNumberOfScales () const
{
    return flow.nscales;
}

void Method::setNumberOfScales (int newValue)
{
    setParameter<int>(flow.nscales, newValue);
}

// Number of warps
int Method::getNumberOfWarps () const
{
    return flow.warps;
}

void Method::setNumberOfWarps (int newValue)
{
    setParameter<int>(flow.warps, newValue);
}

// Epsilon
double Method::getEpsilon () const
{
    return flow.epsilon;
}

void Method::setEpsilon (double newValue)
{
    setParameter<double>(flow.epsilon, newValue);
}

// Number of iterations
int Method::getNumberOfIterations () const
{
    return flow.iterations;
}

void Method::setNumberOfIterations (int newValue)
{
    setParameter<int>(flow.iterations, newValue);
}
