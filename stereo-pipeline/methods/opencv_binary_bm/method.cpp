/*
 * OpenCV Binary-descriptor-based Block Matching: method
 * Copyright (C) 2017 Rok Mandeljc
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
namespace StereoMethodOpenCvBinaryBm {


Method::Method (QObject *parent)
    : QObject(parent), StereoMethod()
{
    resetParameters(); // Will also initialize instance of BM
}

Method::~Method ()
{
}


// *********************************************************************
// *                       StereoMethod interface                      *
// *********************************************************************
QString Method::getShortName () const
{
    return "Binary_BM";
}

QWidget *Method::createConfigWidget (QWidget *parent)
{
    return new MethodWidget(this, parent);
}


// *********************************************************************
// *                              Preset                               *
// *********************************************************************
void Method::resetParameters ()
{
    QMutexLocker locker(&mutex);

    // Reset by creating a new instance
    bm = cv::stereo::StereoBinaryBM::create(64, 5);

    // This method performs additional scaling of disparities, supposedly
    // for better visualization. However, this
    // a) makes it useless for measurement
    // b) reduces the dynamic range, because at the moment, it
    //    internally operates with 8-bit unsigned character type
    // So, always make sure that scaling is disabled!
    bm->setScalleFactor(1);

    locker.unlock();

    emit parameterChanged();
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

    // Compute disparity image
    // NOTE: at the moment, this method works only with CV_8UC1
    tmpDisparity.create(img1.rows, img1.cols, CV_8UC1);

    QMutexLocker locker(&mutex);
    bm->compute(tmpImg1, tmpImg2, tmpDisparity);
    locker.unlock();

    // Normalize to output float format
    switch (tmpDisparity.type()) {
        case CV_16SC1: {
            tmpDisparity.convertTo(disparity, CV_32F, 1/16.0);
            break;
        }
        default: {
            tmpDisparity.convertTo(disparity, CV_32F);
        }
    }

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
    QMutexLocker locker(&mutex);

    bm->setMinDisparity((int)storage["minDisparity"]);
    bm->setNumDisparities((int)storage["numDisparities"]);
    bm->setBlockSize((int)storage["blockSize"]);
    bm->setSpeckleWindowSize((int)storage["speckleWindowSize"]);
    bm->setSpeckleRange((int)storage["speckleRange"]);
    bm->setDisp12MaxDiff((int)storage["disp12MaxDiff"]);

    bm->setPreFilterType((int)storage["preFilterType"]);
    bm->setPreFilterSize((int)storage["preFilterSize"]);
    bm->setPreFilterCap((int)storage["preFilterCap"]);
    bm->setTextureThreshold((int)storage["textureThreshold"]);
    bm->setUniquenessRatio((int)storage["uniquenessRatio"]);
    bm->setSmallerBlockSize((int)storage["smallerBlockSize"]);
    bm->setSpekleRemovalTechnique((int)storage["speckleRemovalTechnique"]);
    bm->setUsePrefilter((int)storage["usePreFilter"]);
    bm->setBinaryKernelType((int)storage["binaryKernelType"]);
    bm->setAgregationWindowSize((int)storage["aggregationWindowSize"]);

    locker.unlock();

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
    storage << "minDisparity" << bm->getMinDisparity();
    storage << "numDisparities" << bm->getNumDisparities();
    storage << "blockSize" << bm->getBlockSize();
    storage << "speckleWindowSize" << bm->getSpeckleWindowSize();
    storage << "speckleRange" << bm->getSpeckleRange();
    storage << "disp12MaxDiff" << bm->getDisp12MaxDiff();

    storage << "preFilterType" << bm->getPreFilterType();
    storage << "preFilterSize" << bm->getPreFilterSize();
    storage << "preFilterCap" << bm->getPreFilterCap();
    storage << "textureThreshold" << bm->getTextureThreshold();
    storage << "uniquenessRatio" << bm->getUniquenessRatio();
    storage << "smallerBlockSize" << bm->getSmallerBlockSize();
    storage << "speckleRemovalTechnique" << bm->getSpekleRemovalTechnique();
    storage << "usePreFilter" << bm->getUsePrefilter();
    storage << "binaryKernelType" << bm->getBinaryKernelType();
    storage << "aggregationWindowSize" << bm->getAgregationWindowSize();
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Minimum disparity
int Method::getMinDisparity () const
{
    return bm->getMinDisparity();
}

void Method::setMinDisparity (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setMinDisparity, value);
}


// Number of disparities
int Method::getNumDisparities () const
{
    return bm->getNumDisparities();
}

void Method::setNumDisparities (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setNumDisparities, value);
}


// Block size
int Method::getBlockSize () const
{
    return bm->getBlockSize();
}

void Method::setBlockSize (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setBlockSize, value);
}


// Speckle window size
int Method::getSpeckleWindowSize () const
{
    return bm->getSpeckleWindowSize();
}


void Method::setSpeckleWindowSize (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setSpeckleWindowSize, value);
}


// Speckle range
int Method::getSpeckleRange () const
{
    return bm->getSpeckleRange();
}


void Method::setSpeckleRange (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setSpeckleRange, value);
}



// Maximum differnece between LR and RL disparity
int Method::getDisp12MaxDiff () const
{
    return bm->getDisp12MaxDiff();
}

void Method::setDisp12MaxDiff (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setDisp12MaxDiff, value);
}



// Use pre-filter
bool Method::getUsePreFilter () const
{
    return bm->getUsePrefilter();
}

void Method::setUsePreFilter (bool value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setUsePrefilter, value);
}


// Pre-filter type
int Method::getPreFilterType () const
{
    return bm->getPreFilterType();
}

void Method::setPreFilterType (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setPreFilterType, value);
}

// Pre-filter type
int Method::getPreFilterSize () const
{
    return bm->getPreFilterSize();
}

void Method::setPreFilterSize (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setPreFilterSize, value);
}

// Pre-filter cap
int Method::getPreFilterCap () const
{
    return bm->getPreFilterCap();
}


void Method::setPreFilterCap (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setPreFilterCap, value);
}


// Texture threshold
int Method::getTextureThreshold () const
{
    return bm->getTextureThreshold();
}


void Method::setTextureThreshold (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setTextureThreshold, value);
}

// Uniqueness ratio
int Method::getUniquenessRatio () const
{
    return bm->getUniquenessRatio();
}


void Method::setUniquenessRatio (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setUniquenessRatio, value);

}


// Smaller block size
int Method::getSmallerBlockSize () const
{
    return bm->getSmallerBlockSize();
}


void Method::setSmallerBlockSize (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setSmallerBlockSize, value);

}


// Speckle removal technique
int Method::getSpeckleRemovalTechnique () const
{
    return bm->getSpekleRemovalTechnique();
}


void Method::setSpeckleRemovalTechnique (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setSpekleRemovalTechnique, value);

}

// Binary kernel type
int Method::getBinaryKernelType () const
{
    return bm->getBinaryKernelType();
}


void Method::setBinaryKernelType (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setBinaryKernelType, value);
}

// Aggregation window size
int Method::getAggregationWindowSize () const
{
    return bm->getAgregationWindowSize();
}


void Method::setAggregationWindowSize (int value)
{
    setParameter(&cv::stereo::StereoBinaryBM::setAgregationWindowSize, value);
}


} // StereoMethodOpenCvBinaryBm
} // Pipeline
} // StereoToolbox
} // MVL
