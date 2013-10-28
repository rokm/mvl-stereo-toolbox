/*
 * OpenCV GPU Dense Optical Flow - Brox: method
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

using namespace StereoMethodFlowBroxGpu;


Method::Method (QObject *parent)
    : QObject(parent), StereoMethod(), flow(0.197f, 0.5f, 0.8f, 10, 77, 10), reverseImages(false)
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
    return "Flow_Brox_GPU";
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
        cv::cvtColor(img1, tmpImg1a, CV_BGR2GRAY);
    } else {
        tmpImg1a = img1;
    }
    tmpImg1a.convertTo(tmpImg1b, CV_32F, 1.0/255.0);

    if (img2.channels() == 3) {
        cv::cvtColor(img2, tmpImg2a, CV_BGR2GRAY);
    } else {
        tmpImg2a = img2;
    }
    tmpImg2a.convertTo(tmpImg2b, CV_32F, 1.0/255.0);

    // Upload to GPU
    gpu_img1.upload(tmpImg1b);
    gpu_img2.upload(tmpImg2b);
    
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
    storage["Alpha"] >> flow.alpha;
    storage["Gamma"] >> flow.gamma;
    storage["ScaleFactor"] >> flow.scale_factor;
    
    storage["InnerIterations"] >> flow.inner_iterations;
    storage["OuterIterations"] >> flow.outer_iterations;
    storage["SolverIterations"] >> flow.solver_iterations;

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
    storage << "ReverseImages" << reverseImages;
    storage << "Alpha" << flow.alpha;
    storage << "Gamma" << flow.gamma;
    storage << "ScaleFactor" << flow.scale_factor;
    
    storage << "InnerIterations" << flow.inner_iterations;
    storage << "OuterIterations" << flow.outer_iterations;
    storage << "SolverIterations" << flow.solver_iterations;
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

// Alpha
double Method::getAlpha () const
{
    return flow.alpha;
}

void Method::setAlpha (double newValue)
{
    setParameter<float>(flow.alpha, newValue);
}

// Gamma
double Method::getGamma () const
{
    return flow.gamma;
}

void Method::setGamma (double newValue)
{
    setParameter<float>(flow.gamma, newValue);
}

// Scale factor
double Method::getScaleFactor () const
{
    return flow.scale_factor;
}

void Method::setScaleFactor (double newValue)
{
    setParameter<float>(flow.scale_factor, newValue);
}

// Inner iterations
int Method::getInnerIterations () const
{
    return flow.inner_iterations;
}

void Method::setInnerIterations (int newValue)
{
    setParameter<int>(flow.inner_iterations, newValue);
}

// Outer iterations
int Method::getOuterIterations () const
{
    return flow.outer_iterations;
}

void Method::setOuterIterations (int newValue)
{
    setParameter<int>(flow.outer_iterations, newValue);
}

// Solver iterations
int Method::getSolverIterations () const
{
    return flow.solver_iterations;
}

void Method::setSolverIterations (int newValue)
{
    setParameter<int>(flow.solver_iterations, newValue);
}