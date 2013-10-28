/*
 * OpenCV GPU Block Matching: method
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

using namespace StereoMethodBlockMatchingGPU;


Method::Method (QObject *parent)
    : QObject(parent), StereoMethod()
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
    return "BM_GPU";
}

QWidget *Method::createConfigWidget (QWidget *parent)
{
    return new MethodWidget(this, parent);
}



// *********************************************************************
// *                             Defaults                              *
// *********************************************************************
void Method::resetToDefaults ()
{
    bm = cv::gpu::StereoBM_GPU();
    emit parameterChanged();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void Method::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    cv::gpu::GpuMat gpu_img1, gpu_img2, gpu_disp;
    
    // Convert to float grayscale
    if (img1.channels() == 3) {
        cv::cvtColor(img1, tmpImg1, CV_BGR2GRAY);
    } else {
        tmpImg1 = img1;
    }

    if (img2.channels() == 3) {
        cv::cvtColor(img2, tmpImg2, CV_BGR2GRAY);
    } else {
        tmpImg2 = img2;
    }

    // Upload to GPU
    gpu_img1.upload(tmpImg1);
    gpu_img2.upload(tmpImg2);

    // Compute disparity image
    QMutexLocker locker(&mutex);
    bm(gpu_img1, gpu_img2, gpu_disp);
    locker.unlock();

    // Download
    gpu_disp.download(disparity);

    // Number of disparities
    numDisparities = getNumDisparities();
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
    bm = cv::gpu::StereoBM_GPU();
    
    storage["Preset"] >> bm.preset;
    storage["NumDisparities"] >> bm.ndisp;
    storage["WindowSize"] >> bm.winSize;
    storage["AverageTextureThreshold"] >> bm.avergeTexThreshold;
    
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
    storage << "Preset" << bm.preset;
    storage << "NumDisparities" << bm.ndisp;
    storage << "WindowSize" << bm.winSize;
    storage << "AverageTextureThreshold" << bm.avergeTexThreshold;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Preset
int Method::getPreset () const
{
    return bm.preset;
}

void Method::setPreset (int newValue)
{
    // Validate
    if (newValue != cv::gpu::StereoBM_GPU::BASIC_PRESET && newValue != cv::gpu::StereoBM_GPU::PREFILTER_XSOBEL) {
        newValue = cv::gpu::StereoBM_GPU::BASIC_PRESET;
    }

    setParameter(bm.preset, newValue);
}

// Number of disparities
int Method::getNumDisparities () const
{
    return bm.ndisp;
}

void Method::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 8.0) * 8; // Must be divisible by 8
    newValue = qMax(8, newValue);

    setParameter(bm.ndisp, newValue);
}
    
// Window size
int Method::getWindowSize () const
{
    return bm.winSize;
}

void Method::setWindowSize (int newValue)
{
    setParameter(bm.winSize, newValue);
}
        

// Average texture threshold
double Method::getAverageTextureThreshold () const
{
    return bm.avergeTexThreshold;
}

void Method::setAverageTextureThreshold (double newValue)
{
    setParameter(bm.avergeTexThreshold, (float)newValue);
}
