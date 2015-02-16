/*
 * Stereo Pipeline: rectification
 * Copyright (C) 2013-2015 Rok Mandeljc
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

#include <stereo-pipeline/rectification.h>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>


#include "rectification_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


Rectification::Rectification (QObject *parent)
    : QObject(parent), d_ptr(new RectificationPrivate(this))
{
}

Rectification::~Rectification ()
{
}


bool Rectification::getState () const
{
    Q_D(const Rectification);
    return d->isValid;
}


const cv::Mat &Rectification::getReprojectionMatrix () const
{
    Q_D(const Rectification);
    return d->Q;
}


const cv::Size &Rectification::getImageSize () const
{
    Q_D(const Rectification);
    return d->imageSize;
}

float Rectification::getStereoBaseline () const
{
    Q_D(const Rectification);

    // Q(3,2) is 1/baseline; units are same as on the pattern, which in
    // our code is millimeters
    if (d->Q.type() == CV_32F) {
        return 1.0 / d->Q.at<float>(3, 2);
    } else {
        return 1.0 / d->Q.at<double>(3, 2);
    }
}


// *********************************************************************
// *                     Calibration import/export                     *
// *********************************************************************
void Rectification::exportStereoCalibration (const QString &filename, const cv::Mat &cameraMatrix1, const cv::Mat &distCoeffs1, const cv::Mat &cameraMatrix2, const cv::Mat &distCoeffs2, const cv::Mat &rotation, const cv::Mat &translation, const cv::Size &imageSize)
{
    // NOTE: we store "raw" parameters, i.e. the ones from which
    // rectification is yet to be computed...
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::WRITE);
    if (storage.isOpened()) {
        storage << "DataType" << "StereoCalibration";

        storage << "M1" << cameraMatrix1;
        storage << "M2" << cameraMatrix2;

        storage << "D1" << distCoeffs1;
        storage << "D2" << distCoeffs2;

        storage << "imageSize" << imageSize;

        storage << "R" << rotation;
        storage << "T" << translation;
    } else {
        throw QString("Failed to open file '%1' for writing!").arg(filename);
    }
}

void Rectification::importStereoCalibration (const QString &filename, cv::Mat &cameraMatrix1, cv::Mat &distCoeffs1, cv::Mat &cameraMatrix2, cv::Mat &distCoeffs2, cv::Mat &rotation, cv::Mat &translation, cv::Size &imageSize)
{
    // Load
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::READ);
    if (!storage.isOpened()) {
        throw QString("Failed to open file '%1' for reading!").arg(filename);
    }

    // Validate data type
    QString dataType = QString::fromStdString(storage["DataType"]);
    if (dataType.compare("StereoCalibration")) {
        throw QString("Invalid stereo calibration data!");
    }

    // Load calibration
    storage["M1"] >> cameraMatrix1;
    storage["M2"] >> cameraMatrix2;
    storage["D1"] >> distCoeffs1;
    storage["D2"] >> distCoeffs2;

    std::vector<int> size;
    storage["imageSize"] >> size;
    imageSize = cv::Size(size[0], size[1]);

    storage["R"] >> rotation;
    storage["T"] >> translation;
}


// *********************************************************************
// *                            Calibration                            *
// *********************************************************************
void Rectification::setStereoCalibration (const cv::Mat &cameraMatrix1, const cv::Mat &distCoeffs1, const cv::Mat &cameraMatrix2, const cv::Mat &distCoeffs2, const cv::Mat &rotation, const cv::Mat &translation, const cv::Size &size)
{
    Q_D(Rectification);

    // Reset state
    d->isValid = false;

    // Set
    d->M1 = cameraMatrix1;
    d->D1 = distCoeffs1;

    d->M2 = cameraMatrix2;
    d->D2 = distCoeffs2;

    d->R = rotation;
    d->T = translation;

    d->imageSize = size;

    // Initialize rectification
    initializeRectification();
}

void Rectification::loadStereoCalibration (const QString &filename)
{
    Q_D(Rectification);

    // Reset state
    d->isValid = false;

    // Import
    importStereoCalibration(filename, d->M1, d->D1, d->M2, d->D2, d->R, d->T, d->imageSize);

    // Initialize rectification from loaded calibration
    initializeRectification();
}

void Rectification::saveStereoCalibration (const QString &filename) const
{
    Q_D(const Rectification);

    // Export
    exportStereoCalibration(filename, d->M1, d->D1, d->M2, d->D2, d->R, d->T, d->imageSize);
}

void Rectification::clearStereoCalibration ()
{
    Q_D(Rectification);

    d->isValid = false;
    d->isVerticalStereo = false;

    emit stateChanged(d->isValid);
}


// *********************************************************************
// *                           Rectification                           *
// *********************************************************************
void Rectification::initializeRectification ()
{
    Q_D(Rectification);

    try {
        cv::stereoRectify(d->M1, d->D1, d->M2, d->D2, d->imageSize, d->R, d->T, d->R1, d->R2, d->P1, d->P2, d->Q, d->zeroDisparity ? cv::CALIB_ZERO_DISPARITY : 0, d->alpha, d->imageSize, &d->validRoi1, &d->validRoi2);
    } catch (...) {
        d->isValid = false;
        emit stateChanged(d->isValid);
        return;
    }

    d->isVerticalStereo = fabs(d->P2.at<double>(1, 3)) > fabs(d->P2.at<double>(0, 3));

    initUndistortRectifyMap(d->M1, d->D1, d->R1, d->P1, d->imageSize, CV_16SC2, d->map11, d->map12);
    initUndistortRectifyMap(d->M2, d->D2, d->R2, d->P2, d->imageSize, CV_16SC2, d->map21, d->map22);

    // Reset ROI
    d->roi = cv::Rect();

    // Change state
    d->isValid = true;
    emit stateChanged(d->isValid);
}


void Rectification::rectifyImagePair (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &img1r, cv::Mat &img2r) const
{
    Q_D(const Rectification);

    // Make sure images are valid
    if (img1.empty() || img2.empty()) {
        return;
    }

    if (!d->isValid || !d->performRectification) {
        // Pass-through
        if (!d->roi.width || !d->roi.height) {
            img1.copyTo(img1r);
            img2.copyTo(img2r);
        } else {
            img1(d->roi).copyTo(img1r);
            img2(d->roi).copyTo(img2r);
        }
    } else {
        if (img1.cols != d->imageSize.width || img1.rows != d->imageSize.height || img2.cols != d->imageSize.width || img2.rows != d->imageSize.height) {
            img1r = cv::Mat();
            img2r = cv::Mat();
            emit error("Input image size does not match calibrated image size!");
            return;
        }

        // Two simple remaps using look-up tables
        if (!d->roi.width || !d->roi.height) {
            // Full maps
            cv::remap(img1, img1r, d->map11, d->map12, cv::INTER_LINEAR);
            cv::remap(img2, img2r, d->map21, d->map22, cv::INTER_LINEAR);
        } else {
            // Subsection of maps
            cv::remap(img1, img1r, d->map11(d->roi), d->map12(d->roi), cv::INTER_LINEAR);
            cv::remap(img2, img2r, d->map21(d->roi), d->map22(d->roi), cv::INTER_LINEAR);
        }
    }
}

// *********************************************************************
// *                                ROI                                *
// *********************************************************************
void Rectification::setRoi (const cv::Rect &newRoi)
{
    Q_D(Rectification);

    if (newRoi != d->roi) {
        d->roi = newRoi;

        emit roiChanged();
    }
}

const cv::Rect &Rectification::getRoi () const
{
    Q_D(const Rectification);
    return d->roi;
}


// *********************************************************************
// *                      Rectification parameters                     *
// *********************************************************************
void Rectification::setZeroDisparity (bool newValue)
{
    Q_D(Rectification);

    if (newValue != d->zeroDisparity) {
        d->zeroDisparity = newValue;

        emit zeroDisparityChanged();

        // Reinitialize rectification
        initializeRectification();
    }
}

bool Rectification::getZeroDisparity () const
{
    Q_D(const Rectification);
    return d->zeroDisparity;
}


void Rectification::setAlpha (float newAlpha)
{
    Q_D(Rectification);

    if (newAlpha != d->alpha) {
        d->alpha = newAlpha;

        emit alphaChanged();

        // Reinitialize rectification
        initializeRectification();
    }
}

float Rectification::getAlpha () const
{
    Q_D(const Rectification);
    return d->alpha;
}


// *********************************************************************
// *                     Perform-rectification flag                    *
// *********************************************************************
void Rectification::setPerformRectification (bool enable)
{
    Q_D(Rectification);

    if (d->performRectification != enable) {
        d->performRectification = enable;

        emit performRectificationChanged(d->performRectification);
    }
}

bool Rectification::getPerformRectification () const
{
    Q_D(const Rectification);
    return d->performRectification;
}


} // Pipeline
} // StereoToolbox
} // MVL
