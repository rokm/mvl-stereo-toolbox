/*
 * Stereo Pipeline: calibration
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

#include "StereoCalibration.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


StereoCalibration::StereoCalibration (QObject *parent)
    : QObject(parent)
{
    isValid = false;

    isVerticalStereo = false;
}

StereoCalibration::~StereoCalibration ()
{
}


bool StereoCalibration::getState () const
{
    // Return state
    return isValid;
}

const cv::Rect &StereoCalibration::getLeftROI () const
{
    return validRoi1;
}

const cv::Rect &StereoCalibration::getRightROI () const
{
    return validRoi2;
}


// *********************************************************************
// *                     Calibration import/export                     * 
// *********************************************************************
void StereoCalibration::loadStereoCalibration (const QString &filename)
{
    // Reset state
    isValid = false;
    //emit stateChanged(isValid);

    // Load
    cv::FileStorage storage(filename.toStdString(), cv::FileStorage::READ);
    if (storage.isOpened()) {
        storage["M1"] >> M1;
        storage["M2"] >> M2;
        storage["D1"] >> D1;
        storage["D2"] >> D2;

        //storage["imageSize"] >> imageSize;
        std::vector<int> size;
        storage["imageSize"] >> size;
        imageSize = cv::Size(size[0], size[1]);
        
        storage["R"] >> R;
        storage["T"] >> T;
        storage["E"] >> E;
        storage["F"] >> F;
    } else {
        throw QString("Failed to open file '%1' for reading!").arg(filename);
    }

    // Initialize rectification from loaded calibration
    initializeStereoRectification();
}

void StereoCalibration::saveStereoCalibration (const QString &filename) const
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
        storage << "E" << E;
        storage << "F" << F;
    } else {
        throw QString("Failed to open file '%1' for writing!").arg(filename);
    }
}

void StereoCalibration::clearStereoCalibration ()
{
    isValid = false;

    isVerticalStereo = false;
    
    emit stateChanged(isValid);
}


// *********************************************************************
// *                           Rectification                           *
// *********************************************************************
void StereoCalibration::rectifyImagePair (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &img1r, cv::Mat &img2r) const
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

void StereoCalibration::initializeStereoRectification ()
{
    cv::stereoRectify(M1, D1, M2, D2, imageSize, R, T, R1, R2, P1, P2, Q, cv::CALIB_ZERO_DISPARITY, 1, imageSize, &validRoi1, &validRoi2);

    isVerticalStereo = fabs(P2.at<double>(1, 3)) > fabs(P2.at<double>(0, 3));

    initUndistortRectifyMap(M1, D1, R1, P1, imageSize, CV_16SC2, map11, map12);
    initUndistortRectifyMap(M2, D2, R2, P2, imageSize, CV_16SC2, map21, map22);

    // Change state
    isValid = true;
    emit stateChanged(isValid);
}


// *********************************************************************
// *                            Calibration                            * 
// *********************************************************************
void StereoCalibration::calibrateFromImages (const QStringList &images, CalibrationPattern &pattern, PatternDetectionValidator *validator)
{
    std::vector<std::vector<cv::Point2f> > imagePoints[2];
    std::vector<std::vector<cv::Point3f> > objectPoints;

    int i, j, k;
    int numImages = images.size() / 2;
    
    // Reset state
    isValid = false;
    emit stateChanged(isValid);

    imageSize = cv::Size();

    // *** Find corners ***
    imagePoints[0].resize(numImages);
    imagePoints[1].resize(numImages);

    // Go over all images
    for (i = j = 0; i < numImages; i++) {
        qDebug() << "Processing pair" << i;
        
        // Left-right pair
        for (k = 0; k < 2; k++) {
            // Load image
            const QString &filename = images[i*2 + k];
            qDebug() << (k == 0 ? "Left" : "Right") << "image:" << filename;

            cv::Mat img = cv::imread(filename.toStdString(), 0);

            if (img.empty()) {
                break;
            }

            // Validate image size
            if (imageSize == cv::Size()) {
                // Store the first ...
                imageSize = img.size();
            } else if (img.size() != imageSize) {
                // ... and validate the rest against it
                qWarning() << "Image" << filename << "has a size that is different from size of the first image; skipping the pair!";
                break;
            }

            // Find calibration pattern in image
            bool found = pattern.findInImage(img, imagePoints[k][j]);

            // Allow user to validate the results, and drop them if
            // necessary
            if (validator) {
                found &= validator->validatePatternDetection(img, imagePoints[k][j]);
            }
            
            // If not found, break the loop (so that if this was the first
            // image of the pair, we do not try the other)
            if (!found) {
                break;
            }
         }

        // If whole image pair was successfully processed, add them to
        // list
        if (k == 2) {
            j++;
        }
    }

    numImages = j;

    qDebug() << "Successfully detected checkboard in" << numImages << "images!";

    if (numImages < 2) {
        throw QString("Too few pairs found to run calibration!");
    }

    // Resize image points
    imagePoints[0].resize(numImages);
    imagePoints[1].resize(numImages);

    // Fill in the world coordinates of pattern points; these are same
    // for all views
    std::vector<cv::Point3f> tmpCoordinates;
    pattern.computePatternCoordinates(tmpCoordinates);

    objectPoints.resize(numImages);
    for (i = 0; i < numImages; i++) {
        objectPoints[i] = tmpCoordinates;
    }

    // *** Calibrate ***
    qDebug() << "Calibrating stereo...";
    
    M1 = cv::Mat::eye(3, 3, CV_64F);
    M2 = cv::Mat::eye(3, 3, CV_64F);

    double rms = stereoCalibrate(objectPoints, imagePoints[0], imagePoints[1],
                                 M1, D1, M2, D2,
                                 imageSize, R, T, E, F,
                                 cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-5),
                                 cv::CALIB_RATIONAL_MODEL);

    qDebug() << "Stereo calibrated; RMS =" << rms << "pixels";

    // *** Compute average reprojection error ***
    double err = 0;
    int num_points = 0;
    std::vector<cv::Vec3f> lines1, lines2;

    for (i = 0; i < numImages; i++) {
        int npt = imagePoints[0][i].size();
        cv::Mat imgpt1, imgpt2;

        // First image
        imgpt1 = cv::Mat(imagePoints[0][i]);
        cv::undistortPoints(imgpt1, imgpt1, M1, D1, cv::Mat(), M1);
        cv::computeCorrespondEpilines(imgpt1, 1, F, lines1);

        // Second image
        imgpt2 = cv::Mat(imagePoints[1][i]);
        cv::undistortPoints(imgpt2, imgpt2, M2, D2, cv::Mat(), M2);
        cv::computeCorrespondEpilines(imgpt2, 2, F, lines2);

        // Compute error
        for (j = 0; j < npt; j++) {
            double errij = fabs(imagePoints[0][i][j].x*lines2[j][0] +
                                imagePoints[0][i][j].y*lines2[j][1] + lines2[j][2]) +
                           fabs(imagePoints[1][i][j].x*lines1[j][0] +
                                imagePoints[1][i][j].y*lines1[j][1] + lines1[j][2]);
            err += errij;
        }

        num_points += npt;
    }

    qDebug() << "Average reprojection error =" <<  err/num_points << "pixels";

    // Initialize rectification from obtained calibration
    initializeStereoRectification();
}


// *********************************************************************
// *                        Calibration pattern                        *
// *********************************************************************
CalibrationPattern::CalibrationPattern (int width, int height, float size, PatternType type, int levels, float increment)
    : patternWidth(width), patternHeight(height), patternSize(width, height),
      elementSize(size), patternType(type),
      maxScaleLevel(levels), scaleIncrement(increment)
{
}

const cv::Size CalibrationPattern::getPatternSize () const
{
    return patternSize;
}


void CalibrationPattern::computePatternCoordinates (std::vector<cv::Point3f> &coordinates) const
{
    coordinates.clear();
    
    switch (patternType) {
        case Chessboard:
        case Circles: {
            // Regular grid
            for (int i = 0; i < patternHeight; i++) {
                for (int j = 0; j < patternWidth; j++) {
                    coordinates.push_back(cv::Point3f(j*elementSize, i*elementSize, 0));
                }
            }
            break;
        }
        case AsymmetricCircles: {
            // Asymmetric grid
            for (int i = 0; i < patternHeight; i++) {
                for (int j = 0; j < patternWidth; j++) {
                    coordinates.push_back(cv::Point3f((2*j + i % 2)*elementSize, i*elementSize, 0));
                }
            }
            break;
        }
    }
}

bool CalibrationPattern::findInImage (const cv::Mat &img, std::vector<cv::Point2f> &points) const
{
    bool found = false;
    
    // Multi-scale search
    for (int scaleLevel = 0; scaleLevel <= maxScaleLevel; scaleLevel++) {
        float scale = 1.0 + scaleLevel*scaleIncrement;
        cv::Mat scaledImg;

        // Rescale image, if necessary
        if (scaleLevel) {
            cv::resize(img, scaledImg, cv::Size(), scale, scale, cv::INTER_CUBIC);
        } else {
            scaledImg = img;
        }

        // Find pattern using OpenCV methods
        switch (patternType) {
            case Chessboard: {
                found = cv::findChessboardCorners(scaledImg, patternSize, points, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE /*| cv::CALIB_CB_FAST_CHECK*/);
                break;
            }
            case Circles: {
                found = cv::findCirclesGrid(scaledImg, patternSize, points);
                break;
            }
            case AsymmetricCircles: {
                found = cv::findCirclesGrid(scaledImg, patternSize, points, cv::CALIB_CB_ASYMMETRIC_GRID);
                break;
            }
        }
        
        if (found) {
            // Improve localization of corners on chessboard by doing
            // sub-pixel interpolation
            if (patternType == Chessboard) {
                cv::cornerSubPix(img, points, cv::Size(11,11), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01));
            }
            
            // If points were found at higher scale, scale them back to
            // the original image... We do this after sub-pixel localization
            // (in case of chessboard), because sub-pixel localization can
            // fail miserably at original image size (where checkboard was
            // not detected in first place!)
            if (scaleLevel) {
                cv::Mat pointsMat(points); // Construct matrix with shared data...
                pointsMat *= 1.0/scale;
            }
            
            break;
        }
    }

    return found;
}


