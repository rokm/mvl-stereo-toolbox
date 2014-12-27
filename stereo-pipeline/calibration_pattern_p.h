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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__CALIBRATION_PATTERN_P_H
#define MVL_STEREO_TOOLBOX__PIPELINE__CALIBRATION_PATTERN_P_H


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {


class CalibrationPatternPrivate
{
    Q_DECLARE_PUBLIC(CalibrationPattern)

    CalibrationPattern * const q_ptr;

    CalibrationPatternPrivate (CalibrationPattern *);

protected:
    int patternWidth;
    int patternHeight;
    cv::Size patternSize;

    float elementSize;

    CalibrationPattern::PatternType patternType;

    int maxScaleLevel;
    float scaleIncrement;
};


CalibrationPatternPrivate::CalibrationPatternPrivate (CalibrationPattern *pattern)
    : q_ptr(pattern)
{
}


} // Pipeline
} // StereoToolbox
} // MVL


#endif
