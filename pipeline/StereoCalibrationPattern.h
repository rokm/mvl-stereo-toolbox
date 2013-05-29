/*
 * Stereo Pipeline: calibration pattern
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

#ifndef STEREO_CALIBRATION_PATTERN_H
#define STEREO_CALIBRATION_PATTERN_H

#include <opencv2/core/core.hpp>


class StereoCalibrationPattern
{
public:
    enum PatternType {
        Chessboard,
        Circles,
        AsymmetricCircles,
    };

    StereoCalibrationPattern ();
    StereoCalibrationPattern (int, int, float, PatternType, int = 0, float = 0.25);

    void setParameters (int, int, float, PatternType, int = 0, float = 0.25);

    const cv::Size getPatternSize () const;

    std::vector<cv::Point3f> computePlanarCoordinates () const;

    bool findInImage (const cv::Mat &, std::vector<cv::Point2f> &) const;
    
protected:
    int patternWidth;
    int patternHeight;
    cv::Size patternSize;

    float elementSize;
    
    PatternType patternType;

    int maxScaleLevel;
    float scaleIncrement;
};

#endif