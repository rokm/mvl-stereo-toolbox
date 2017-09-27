/*
 * MVL Stereo Toolbox: calibration wizard
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

#include "wizard.h"

#include "page_introduction.h"
#include "page_images.h"
#include "page_detection.h"
#include "page_calibration.h"
#include "page_result.h"


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


Wizard::Wizard (QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle("Calibration Wizard");
    setWizardStyle(ClassicStyle);

    resize(800, 600);

    setOption(QWizard::NoBackButtonOnStartPage, true);

    // Register metatypes for OpenCV types that are passed between pages
    qRegisterMetaType< std::vector<std::vector<cv::Point2f> > >();
    qRegisterMetaType< std::vector<std::vector<cv::Point3f> > >();
    qRegisterMetaType< cv::Size >();
    qRegisterMetaType< cv::Mat >();

    // *** Pages ***
    setPage(PageId::IntroductionId, new PageIntroduction(this));

    setPage(PageId::StereoImagesId, new PageStereoImages(this));
    setPage(PageId::StereoDetectionId, new PageStereoDetection(this));
    setPage(PageId::StereoCalibrationId, new PageStereoCalibration(this));
    setPage(PageId::StereoResultId, new PageStereoResult(this));

    setPage(PageId::LeftCameraImagesId, new PageLeftCameraImages(this));
    setPage(PageId::LeftCameraDetectionId, new PageLeftCameraDetection(this));
    setPage(PageId::LeftCameraCalibrationId, new PageLeftCameraCalibration(this));
    setPage(PageId::LeftCameraResultId, new PageLeftCameraResult(this));
    setPage(PageId::RightCameraImagesId, new PageRightCameraImages(this));
    setPage(PageId::RightCameraDetectionId, new PageRightCameraDetection(this));
    setPage(PageId::RightCameraCalibrationId, new PageRightCameraCalibration(this));
    setPage(PageId::RightCameraResultId, new PageRightCameraResult(this));

    setPage(PageId::SingleCameraImagesId, new PageSingleCameraImages(this));
    setPage(PageId::SingleCameraDetectionId, new PageSingleCameraDetection(this));
    setPage(PageId::SingleCameraCalibrationId, new PageSingleCameraCalibration(this));
    setPage(PageId::SingleCameraResultId, new PageSingleCameraResult(this));

    setStartId(PageId::IntroductionId);
}

Wizard::~Wizard()
{
}


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL
