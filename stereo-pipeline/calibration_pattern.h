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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__CALIBRATION_PATTERN_H
#define MVL_STEREO_TOOLBOX__PIPELINE__CALIBRATION_PATTERN_H

#include <stereo-pipeline/export.h>

#include <QtCore>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class CalibrationPatternPrivate;

class MVL_STEREO_PIPELINE_EXPORT CalibrationPattern
{
    Q_DECLARE_PRIVATE(CalibrationPattern)
    QScopedPointer<CalibrationPatternPrivate> const d_ptr;

public:
    enum PatternType {
        Chessboard,
        Circles,
        AsymmetricCircles,
    };

    CalibrationPattern ();
    CalibrationPattern (int patternWidth, int patternHeight, float elementSize, PatternType patternType, int maxScaleLevel = 0, float scaleIncrement = 0.25f);
    virtual ~CalibrationPattern ();

    void setParameters (int patternWidth, int patternHeight, float elementSize, PatternType patternType, int maxScaleLevel = 0, float scaleIncrement = 0.25f);

    const cv::Size getPatternSize () const;

    std::vector<cv::Point3f> computePlanarCoordinates () const;

    bool findInImage (const cv::Mat &img, std::vector<cv::Point2f> &points) const;
};


} // Pipeline
} // StereoToolbox
} // MVL


#endif
