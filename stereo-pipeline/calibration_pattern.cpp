/*
 * Stereo Pipeline: calibration pattern
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

#include <stereo-pipeline/calibration_pattern.h>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>


#include "calibration_pattern_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


CalibrationPatternPrivate::CalibrationPatternPrivate (CalibrationPattern *parent)
    : q_ptr(parent)
{
}


CalibrationPattern::CalibrationPattern ()
    : d_ptr(new CalibrationPatternPrivate(this))
{
}

CalibrationPattern::CalibrationPattern (int patternWidth, int patternHeight, float elementSize, PatternType patternType, int maxScaleLevel, float scaleIncrement)
    : d_ptr(new CalibrationPatternPrivate(this))
{
    setParameters(patternWidth, patternHeight, elementSize, patternType, maxScaleLevel, scaleIncrement);
}

CalibrationPattern::~CalibrationPattern ()
{
}


// *********************************************************************
// *                            Parameters                             *
// *********************************************************************
void CalibrationPattern::setParameters (int patternWidth, int patternHeight, float elementSize, PatternType patternType, int maxScaleLevel, float scaleIncrement)
{
    Q_D(CalibrationPattern);

    d->patternWidth = patternWidth;
    d->patternHeight = patternHeight;
    d->patternSize = cv::Size(d->patternWidth, d->patternHeight);

    d->elementSize = elementSize;

    d->patternType = patternType;

    d->maxScaleLevel = maxScaleLevel;
    d->scaleIncrement = scaleIncrement;
}


const cv::Size CalibrationPattern::getPatternSize () const
{
    Q_D(const CalibrationPattern);
    return d->patternSize;
}


// *********************************************************************
// *            Coordinates of pattern on calibration plane            *
// *********************************************************************
std::vector<cv::Point3f> CalibrationPattern::computePlanarCoordinates () const
{
    Q_D(const CalibrationPattern);
    std::vector<cv::Point3f> coordinates;

    switch (d->patternType) {
        case Chessboard:
        case Circles: {
            // Regular grid
            for (int i = 0; i < d->patternHeight; i++) {
                for (int j = 0; j < d->patternWidth; j++) {
                    coordinates.push_back(cv::Point3f(j*d->elementSize, i*d->elementSize, 0));
                }
            }
            break;
        }
        case AsymmetricCircles: {
            // Asymmetric grid
            for (int i = 0; i < d->patternHeight; i++) {
                for (int j = 0; j < d->patternWidth; j++) {
                    coordinates.push_back(cv::Point3f((2*j + i % 2)*d->elementSize, i*d->elementSize, 0));
                }
            }
            break;
        }
    }

    return coordinates;
}


// *********************************************************************
// *                         Pattern detection                         *
// *********************************************************************
bool CalibrationPattern::findInImage (const cv::Mat &img, std::vector<cv::Point2f> &points) const
{
    Q_D(const CalibrationPattern);
    bool found = false;

    // Multi-scale search
    for (int scaleLevel = 0; scaleLevel <= d->maxScaleLevel; scaleLevel++) {
        float scale = 1.0 + scaleLevel*d->scaleIncrement;
        cv::Mat scaledImg;

        // Rescale image, if necessary
        if (scaleLevel) {
            cv::resize(img, scaledImg, cv::Size(), scale, scale, cv::INTER_CUBIC);
        } else {
            scaledImg = img;
        }

        // Find pattern using OpenCV methods
        switch (d->patternType) {
            case Chessboard: {
                found = cv::findChessboardCorners(scaledImg, d->patternSize, points, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE /*| cv::CALIB_CB_FAST_CHECK*/);
                break;
            }
            case Circles: {
                found = cv::findCirclesGrid(scaledImg, d->patternSize, points);
                break;
            }
            case AsymmetricCircles: {
                found = cv::findCirclesGrid(scaledImg, d->patternSize, points, cv::CALIB_CB_ASYMMETRIC_GRID);
                break;
            }
        }

        if (found) {
            // Improve localization of corners on chessboard by doing
            // sub-pixel interpolation
            if (d->patternType == Chessboard) {
                cv::Mat scaledImgGray;
                if (scaledImg.channels() == 1) {
                    scaledImgGray = scaledImg;
                } else {
                    cv::cvtColor(scaledImg, scaledImgGray, cv::COLOR_BGR2GRAY);
                }

                cv::cornerSubPix(scaledImgGray, points, cv::Size(11,11), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01));
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


} // Pipeline
} // StereoToolbox
} // MVL
