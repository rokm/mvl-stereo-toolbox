/*
 * MVL Stereo Toolbox: calibration GUI: calibration pattern detection dialog
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

#ifndef CALIBRATION_PATTERN_DETECTION_DIALOG_H
#define CALIBRATION_PATTERN_DETECTION_DIALOG_H

#include <QtGui>
#include <opencv2/core/core.hpp>


class CalibrationPatternDisplayWidget;

class CalibrationPatternDetectionDialog : public QDialog
{
public:
    CalibrationPatternDetectionDialog (QWidget * = 0);
    virtual ~CalibrationPatternDetectionDialog ();

    void setImage (const cv::Mat &, bool, const std::vector<cv::Point2f> &, const cv::Size &);

protected:
    CalibrationPatternDisplayWidget *displayImage;
    QPushButton *pushButtonAccept;
    QPushButton *pushButtonDiscard;
};

#endif
