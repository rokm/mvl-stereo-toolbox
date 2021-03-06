/*
 * Stereo Pipeline: rectification
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

#include "rectification.h"
#include "exception.h"

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>


#include "rectification_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


RectificationPrivate::RectificationPrivate (Rectification *parent)
    : q_ptr(parent)
{
    isValid = false;
    performRectification = true;

    isVerticalStereo = false;

    alpha = 0;
    zeroDisparity = true;
}


Rectification::Rectification (QObject *parent)
    : QObject(parent), d_ptr(new RectificationPrivate(this))
{
}

Rectification::~Rectification ()
{
}


// *********************************************************************
// *                               State                               *
// *********************************************************************
bool Rectification::isCalibrationValid () const
{
    Q_D(const Rectification);
    return d->isValid;
}


// *********************************************************************
// *                     Calibration import/export                     *
// *********************************************************************
void Rectification::exportStereoCalibration (const QString &filename, const cv::Mat &cameraMatrix1, const cv::Mat &distCoeffs1, const cv::Mat &cameraMatrix2, const cv::Mat &distCoeffs2, const cv::Mat &rotation, const cv::Mat &translation, const cv::Size &imageSize, bool zeroDisparity, double alpha)
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

        storage << "zeroDisparity" << zeroDisparity;
        storage << "alpha" << alpha;
    } else {
        throw Exception(QStringLiteral("Failed to open file '%1' for writing!").arg(filename));
    }
}

void Rectification::importStereoCalibration (const QString &filename, cv::Mat &cameraMatrix1, cv::Mat &distCoeffs1, cv::Mat &cameraMatrix2, cv::Mat &distCoeffs2, cv::Mat &rotation, cv::Mat &translation, cv::Size &imageSize, bool &zeroDisparity, double &alpha)
{
    // Load
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::READ);
    if (!storage.isOpened()) {
        throw Exception(QStringLiteral("Failed to open file '%1' for reading!").arg(filename));
    }

    // Validate data type
    QString dataType = QString::fromStdString(storage["DataType"]);
    if (dataType.compare("StereoCalibration")) {
        throw Exception(QStringLiteral("Invalid stereo calibration data!"));
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

    // Load rectification paramaters (optional)
    cv::FileNode node = storage["zeroDisparity"];
    if (!node.isNone()) {
        node >> zeroDisparity;
    } else {
        zeroDisparity = true;
    }

    node = storage["alpha"];
    if (!node.isNone()) {
        node >> alpha;
    } else {
        alpha = 0.0;
    }
}


// *********************************************************************
// *                            Calibration                            *
// *********************************************************************
void Rectification::setStereoCalibration (const cv::Mat &cameraMatrix1, const cv::Mat &distCoeffs1, const cv::Mat &cameraMatrix2, const cv::Mat &distCoeffs2, const cv::Mat &rotation, const cv::Mat &translation, const cv::Size &imageSize, bool zeroDisparity, double alpha)
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

    d->imageSize = imageSize;

    d->zeroDisparity = zeroDisparity;
    d->alpha = alpha;

    // Initialize rectification
    initializeRectification();
}

void Rectification::loadStereoCalibration (const QString &filename)
{
    Q_D(Rectification);

    // Reset state
    d->isValid = false;

    // Import
    importStereoCalibration(filename, d->M1, d->D1, d->M2, d->D2, d->R, d->T, d->imageSize, d->zeroDisparity, d->alpha);

    // Initialize rectification from loaded calibration
    initializeRectification();
}

void Rectification::saveStereoCalibration (const QString &filename) const
{
    Q_D(const Rectification);

    // Export
    exportStereoCalibration(filename, d->M1, d->D1, d->M2, d->D2, d->R, d->T, d->imageSize, d->zeroDisparity, d->alpha);
}

void Rectification::clearStereoCalibration ()
{
    Q_D(Rectification);

    d->isValid = false;
    d->isVerticalStereo = false;

    emit calibrationChanged(d->isValid);
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
        emit calibrationChanged(d->isValid);
        return;
    }

    d->isVerticalStereo = fabs(d->P2.at<double>(1, 3)) > fabs(d->P2.at<double>(0, 3));

    initUndistortRectifyMap(d->M1, d->D1, d->R1, d->P1, d->imageSize, CV_16SC2, d->map11, d->map12);
    initUndistortRectifyMap(d->M2, d->D2, d->R2, d->P2, d->imageSize, CV_16SC2, d->map21, d->map22);

    // Change state
    d->isValid = true;
    emit calibrationChanged(d->isValid);
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
        img1.copyTo(img1r);
        img2.copyTo(img2r);
    } else {
        if (img1.cols != d->imageSize.width || img1.rows != d->imageSize.height || img2.cols != d->imageSize.width || img2.rows != d->imageSize.height) {
            img1r = cv::Mat();
            img2r = cv::Mat();
            emit error("Input image size does not match calibrated image size!");
            return;
        }

        // Two simple remaps using look-up tables
        // Full maps
        cv::remap(img1, img1r, d->map11, d->map12, cv::INTER_LINEAR);
        cv::remap(img2, img2r, d->map21, d->map22, cv::INTER_LINEAR);
    }
}


// *********************************************************************
// *                      Rectification parameters                     *
// *********************************************************************
void Rectification::setZeroDisparity (bool enable)
{
    Q_D(Rectification);

    if (enable != d->zeroDisparity) {
        d->zeroDisparity = enable;

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


void Rectification::setAlpha (float alpha)
{
    Q_D(Rectification);

    if (alpha != d->alpha) {
        d->alpha = alpha;

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


// *********************************************************************
// *                  Individual calibration elements                  *
// *********************************************************************
const cv::Size &Rectification::getImageSize () const
{
    Q_D(const Rectification);
    return d->imageSize;
}

const cv::Mat &Rectification::getCameraMatrix1 () const
{
    Q_D(const Rectification);
    return d->M1;
}

const cv::Mat &Rectification::getDistortionCoefficients1 () const
{
    Q_D(const Rectification);
    return d->D1;
}


const cv::Mat &Rectification::getCameraMatrix2 () const
{
    Q_D(const Rectification);
    return d->M2;
}

const cv::Mat &Rectification::getDistortionCoefficients2 () const
{
    Q_D(const Rectification);
    return d->D2;
}


const cv::Mat &Rectification::getRotationBetweenCameras () const
{
    Q_D(const Rectification);
    return d->R;
}

const cv::Mat &Rectification::getTranslationBetweenCameras () const
{
    Q_D(const Rectification);
    return d->T;
}

const cv::Mat &Rectification::getEssentialMatrix () const
{
    Q_D(const Rectification);
    return d->E;
}

const cv::Mat &Rectification::getFundamentalMatrix () const
{
    Q_D(const Rectification);
    return d->F;
}


// *********************************************************************
// *                 Individual rectification elements                 *
// *********************************************************************
const cv::Mat &Rectification::getRectificationTransformMatrix1 () const
{
    Q_D(const Rectification);
    return d->R1;
}

const cv::Mat &Rectification::getRectifiedCameraMatrix1 () const
{
    Q_D(const Rectification);
    return d->P1;
}


const cv::Mat &Rectification::getRectificationTransformMatrix2 () const
{
    Q_D(const Rectification);
    return d->R2;
}

const cv::Mat &Rectification::getRectifiedCameraMatrix2 () const
{
    Q_D(const Rectification);
    return d->P2;
}


const cv::Mat &Rectification::getReprojectionMatrix () const
{
    Q_D(const Rectification);
    return d->Q;
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


} // Pipeline
} // StereoToolbox
} // MVL
