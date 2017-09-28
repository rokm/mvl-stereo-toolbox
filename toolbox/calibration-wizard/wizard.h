/*
 * MVL Stereo Toolbox: calibration wizard
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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__WIZARD_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__WIZARD_H

#include <QtWidgets>

#include <opencv2/core.hpp>


// Metatype declarations for OpenCV types, so we can pass them between
// wizard pages
Q_DECLARE_METATYPE(std::vector<std::vector<cv::Point2f> >);
Q_DECLARE_METATYPE(std::vector<std::vector<cv::Point3f> >);
Q_DECLARE_METATYPE(cv::Size);
Q_DECLARE_METATYPE(cv::Mat);


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                               Wizard                              *
// *********************************************************************
class Wizard : public QWizard
{
    Q_OBJECT

public:
    Wizard (QWidget *parent = Q_NULLPTR);
    virtual ~Wizard ();

    enum PageId {
        IntroductionId,
        LeftCameraImagesId,
        LeftCameraDetectionId,
        LeftCameraCalibrationId,
        LeftCameraResultId,
        RightCameraImagesId,
        RightCameraDetectionId,
        RightCameraCalibrationId,
        RightCameraResultId,
        StereoImagesId,
        StereoDetectionId,
        StereoCalibrationId,
        StereoResultId,
        SingleCameraImagesId,
        SingleCameraDetectionId,
        SingleCameraCalibrationId,
        SingleCameraResultId,
    };
};


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL


#endif
