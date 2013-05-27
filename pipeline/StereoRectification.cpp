/*
 * Stereo Pipeline: stereo rectification
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

#include "StereoRectification.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>


StereoRectification::StereoRectification (QObject *parent)
    : QObject(parent)
{
    isValid = false;

    isVerticalStereo = false;
}

StereoRectification::~StereoRectification ()
{
}


bool StereoRectification::getState () const
{
    // Return state
    return isValid;
}

const cv::Rect &StereoRectification::getLeftROI () const
{
    return validRoi1;
}

const cv::Rect &StereoRectification::getRightROI () const
{
    return validRoi2;
}


// *********************************************************************
// *                            Calibration                            *
// *********************************************************************
void StereoRectification::setStereoCalibration (const cv::Mat &cameraMatrix1, const cv::Mat &distCoeffs1, const cv::Mat &cameraMatrix2, const cv::Mat &distCoeffs2, const cv::Mat &rotation, const cv::Mat &translation, const cv::Size &size)
{
    // Reset state
    isValid = false;

    // Set
    M1 = cameraMatrix1;
    D1 = distCoeffs1;

    M2 = cameraMatrix2;
    D2 = distCoeffs2;

    R = rotation;
    T = translation;

    imageSize = size;

    // Initialize rectification
    initializeStereoRectification();
}

void StereoRectification::loadStereoCalibration (const QString &filename)
{
    // Reset state
    isValid = false;

    // Load
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::READ);
    if (storage.isOpened()) {
        storage["M1"] >> M1;
        storage["M2"] >> M2;
        storage["D1"] >> D1;
        storage["D2"] >> D2;

        std::vector<int> size;
        storage["imageSize"] >> size;
        imageSize = cv::Size(size[0], size[1]);
        
        storage["R"] >> R;
        storage["T"] >> T;
    } else {
        throw QString("Failed to open file '%1' for reading!").arg(filename);
    }

    // Initialize rectification from loaded calibration
    initializeStereoRectification();
}

void StereoRectification::saveStereoCalibration (const QString &filename) const
{
    // NOTE: we store "raw" parameters, i.e. the ones from which
    // rectification is yet to be computed...
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::WRITE);
    if (storage.isOpened()) {
        storage << "M1" << M1;
        storage << "M2" << M2;

        storage << "D1" << D1;
        storage << "D2" << D2;

        storage << "imageSize" << imageSize;
        
        storage << "R" << R;
        storage << "T" << T;
    } else {
        throw QString("Failed to open file '%1' for writing!").arg(filename);
    }
}

void StereoRectification::clearStereoCalibration ()
{
    isValid = false;
    isVerticalStereo = false;
    
    emit stateChanged(isValid);
}


// *********************************************************************
// *                           Rectification                           *
// *********************************************************************
void StereoRectification::initializeStereoRectification ()
{
    try {
        cv::stereoRectify(M1, D1, M2, D2, imageSize, R, T, R1, R2, P1, P2, Q, cv::CALIB_ZERO_DISPARITY, 1, imageSize, &validRoi1, &validRoi2);
    } catch (...) {
        isValid = false;
        emit stateChanged(isValid);
        return;
    }

    isVerticalStereo = fabs(P2.at<double>(1, 3)) > fabs(P2.at<double>(0, 3));

    initUndistortRectifyMap(M1, D1, R1, P1, imageSize, CV_16SC2, map11, map12);
    initUndistortRectifyMap(M2, D2, R2, P2, imageSize, CV_16SC2, map21, map22);

    // Change state
    isValid = true;
    emit stateChanged(isValid);
}


void StereoRectification::rectifyImagePair (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &img1r, cv::Mat &img2r) const
{
    // Make sure images are valid
    if (!img1.data || !img2.data) {
        return;
    }
    
    if (!isValid) {
        // Pass-through
        img1.copyTo(img1r);
        img2.copyTo(img2r);
    } else {
        // Two simple remaps using look-up tables
        cv::remap(img1, img1r, map11, map12, cv::INTER_LINEAR);
        cv::remap(img2, img2r, map21, map22, cv::INTER_LINEAR);
    }
}

