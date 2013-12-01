/*
 * OpenCV GPU Dense Optical Flow - Lukas-Kanade with pyramids: method
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

using namespace StereoMethodFlowPyrLKGpu;


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
    return "Flow_PyrLK_GPU";
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
        flow.dense(gpu_img1, gpu_img2, gpu_disp_u, gpu_disp_v);
    } else {
        flow.dense(gpu_img2, gpu_img1, gpu_disp_u, gpu_disp_v);
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
    storage["WindowWidth"] >> flow.winSize.width;
    storage["WindowHeight"] >> flow.winSize.height;
    storage["MaxLevel"] >> flow.maxLevel;
    storage["NumberOfIterations"] >> flow.iters;
    
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
    storage << "WindowWidth" <<  flow.winSize.width;
    storage << "WindowHeight" <<  flow.winSize.height;
    storage << "MaxLevel" <<  flow.maxLevel;
    storage << "NumberOfIterations"  << flow.iters;
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

// Window width
int Method::getWindowWidth () const
{
    return flow.winSize.width;
}

void Method::setWindowWidth (int newValue)
{
    setParameter<int>(flow.winSize.width, newValue);
}

// Window height
int Method::getWindowHeight () const
{
    return flow.winSize.height;
}

void Method::setWindowHeight (int newValue)
{
    setParameter<int>(flow.winSize.height, newValue);
}

// Max level
int Method::getMaxLevel () const
{
    return flow.maxLevel;
}

void Method::setMaxLevel (int newValue)
{
    setParameter<int>(flow.maxLevel, newValue);
}

// Number of iterations
int Method::getNumberOfIterations () const
{
    return flow.iters;
}

void Method::setNumberOfIterations (int newValue)
{
    setParameter<int>(flow.iters, newValue);
}
