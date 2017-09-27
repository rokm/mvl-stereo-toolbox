/*
 * MVL Stereo Toolbox: calibration wizard: result page
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

#include "page_result.h"
#include "wizard.h"
#include "camera_parameters_widget.h"

#include <stereo-pipeline/rectification.h>
#include <stereo-widgets/image_display_widget.h>
#include <stereo-widgets/image_pair_display_widget.h>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {



// *********************************************************************
// *                        Result page: common                        *
// *********************************************************************
PageResult::PageResult (const QString &fieldPrefixString, QWidget *parent)
    : QWizardPage(parent),
      fieldPrefix(fieldPrefixString)
{
    setSubTitle("Calibration result");

    QLabel *label;

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Congratulations! Camera's intrinsic parameters have been calibrated. One of calibration images has been undistorted and is shown below for visual validation of calibration result.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label);

    // Splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(splitter);

    // Camera parameters
    widgetCameraParameters = new CameraParametersWidget("Parameters", this);
    widgetCameraParameters->setDisplayMode(true);
    splitter->addWidget(widgetCameraParameters);

    // Undistorted image
    widgetImage = new Widgets::ImageDisplayWidget("Undistorted image", this);
    widgetImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widgetImage->resize(400, 600);
    splitter->addWidget(widgetImage);
}

PageResult::~PageResult ()
{
}

void PageResult::initializePage ()
{
    // Get camera calibration
    cv::Mat M = field(fieldPrefix + "CameraMatrix").value<cv::Mat>();
    cv::Mat D = field(fieldPrefix + "DistCoeffs").value<cv::Mat>();

    // Display parameters
    widgetCameraParameters->setCameraMatrix(M, D);

    // Load and undistort first image
    QStringList images = field(fieldPrefix + "Images").toStringList();

    cv::Mat image = cv::imread(images[0].toStdString(), -1);
    cv::Mat undistortedImage;

    cv::undistort(image, undistortedImage, M, D);

    // Display undistorted image
    widgetImage->setImage(undistortedImage);
}

void PageResult::setVisible (bool visible)
{
    QWizardPage::setVisible(visible);

    // On Windows, this function gets called without wizard being set...
    if (!wizard()) {
        return;
    }

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("&Export"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        customButtonConnection = connect(wizard(), &QWizard::customButtonClicked, this, &PageResult::exportCalibration);
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        QObject::disconnect(customButtonConnection);
    }
}

void PageResult::exportCalibration ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save camera calibration to file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        QString ext = QFileInfo(fileName).completeSuffix();
        if (ext.isEmpty()) {
            ext = "yml";
            fileName += "." + ext;
        }

        // Get parameters
        cv::Mat cameraMatrix = field(fieldPrefix + "CameraMatrix").value<cv::Mat>();
        cv::Mat distCoeffs = field(fieldPrefix + "DistCoeffs").value<cv::Mat>();
        cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();

        // Store
        cv::FileStorage storage(fileName.toStdString(), cv::FileStorage::WRITE);
        if (storage.isOpened()) {
            storage << "DataType" << "CameraCalibration";
            storage << "imageSize" << imageSize;
            storage << "cameraMatrix" << cameraMatrix;
            storage << "distCoeffs" << distCoeffs;
        } else {
            QMessageBox::warning(this, "Error", QString("Failed to export calibration to '%1'").arg(fileName));
        }
    }
}


// *********************************************************************
// *                     Result page: left camera                      *
// *********************************************************************
PageLeftCameraResult::PageLeftCameraResult (QWidget *parent)
    : PageResult("LeftCamera", parent)
{
    setTitle("Left camera calibration");
}

PageLeftCameraResult::~PageLeftCameraResult ()
{
}

int PageLeftCameraResult::nextId () const
{
    return Wizard::PageId::RightCameraImagesId;
}


// *********************************************************************
// *                     Result page: right camera                     *
// *********************************************************************
PageRightCameraResult::PageRightCameraResult (QWidget *parent)
    : PageResult("RightCamera", parent)
{
    setTitle("Right camera calibration");
}

PageRightCameraResult::~PageRightCameraResult ()
{
}

int PageRightCameraResult::nextId () const
{
    return Wizard::PageId::StereoImagesId;
}


// *********************************************************************
// *                        Result page: stereo                        *
// *********************************************************************
PageStereoResult::PageStereoResult (QWidget *parent)
    : QWizardPage(parent),
      fieldPrefix("Stereo")
{
    setTitle("Stereo calibration");
    setSubTitle("Calibration result");

    QLabel *label;

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Congratulations! Stereo parameters have been calibrated. One of calibration image pairs has been rectified and is shown below for visual validation of calibration result.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label);

    // Splitter
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(splitter);

    // Undistorted image
    widgetImage = new Widgets::ImagePairDisplayWidget("Rectified image pair", this);
    widgetImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widgetImage->resize(400, 600);
    splitter->addWidget(widgetImage);

    // Scroll area; camera parameters
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);

    QWidget *parametersWidget = new QWidget(scrollArea);
    QHBoxLayout *boxLayout = new QHBoxLayout(parametersWidget);
    parametersWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    boxLayout->setContentsMargins(0, 0, 0, 0);

    // Left camera parameters
    widgetLeftCameraParameters = new CameraParametersWidget("Left camera", this);
    widgetLeftCameraParameters->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widgetLeftCameraParameters->setDisplayMode(true);
    boxLayout->addWidget(widgetLeftCameraParameters);

    //boxLayout->addStretch();

    // Right camera parameters
    widgetRightCameraParameters = new CameraParametersWidget("Right camera", this);
    widgetRightCameraParameters->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widgetRightCameraParameters->setDisplayMode(true);
    boxLayout->addWidget(widgetRightCameraParameters);

    scrollArea->setWidget(parametersWidget);
    splitter->addWidget(scrollArea);
}

PageStereoResult::~PageStereoResult ()
{
}

void PageStereoResult::initializePage ()
{
    // Get stereo calibration
    cv::Mat M1 = field(fieldPrefix + "CameraMatrix1").value<cv::Mat>();
    cv::Mat D1 = field(fieldPrefix + "DistCoeffs1").value<cv::Mat>();
    cv::Mat M2 = field(fieldPrefix + "CameraMatrix2").value<cv::Mat>();
    cv::Mat D2 = field(fieldPrefix + "DistCoeffs2").value<cv::Mat>();
    cv::Mat T = field(fieldPrefix + "T").value<cv::Mat>();
    cv::Mat R = field(fieldPrefix + "R").value<cv::Mat>();
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();

    // Display parameters
    widgetLeftCameraParameters->setCameraMatrix(M1, D1);
    widgetRightCameraParameters->setCameraMatrix(M2, D2);

    // Initialize stereo rectification
    cv::Mat R1, R2;
    cv::Mat P1, P2;
    cv::Mat Q;
    cv::Rect validRoi1, validRoi2;
    cv::Mat map11, map12, map21, map22;

    cv::stereoRectify(M1, D1, M2, D2, imageSize, R, T, R1, R2, P1, P2, Q, cv::CALIB_ZERO_DISPARITY, 0, imageSize, &validRoi1, &validRoi2);

    cv::initUndistortRectifyMap(M1, D1, R1, P1, imageSize, CV_16SC2, map11, map12);
    cv::initUndistortRectifyMap(M2, D2, R2, P2, imageSize, CV_16SC2, map21, map22);

    // Load and rectify a pair
    QStringList images = field(fieldPrefix + "Images").toStringList();

    cv::Mat image1 = cv::imread(images[0].toStdString(), -1);
    cv::Mat image2 = cv::imread(images[1].toStdString(), -1);

    // Two simple remaps using look-up tables
    cv::Mat rectifiedImage1, rectifiedImage2;

    cv::remap(image1, rectifiedImage1, map11, map12, cv::INTER_LINEAR);
    cv::remap(image2, rectifiedImage2, map21, map22, cv::INTER_LINEAR);

    // Display undistorted image
    widgetImage->setImagePair(rectifiedImage1, rectifiedImage2);
    widgetImage->setImagePairROI(validRoi1, validRoi2);
}

void PageStereoResult::setVisible (bool visible)
{
    QWizardPage::setVisible(visible);

    // On Windows, this function gets called without wizard being set...
    if (!wizard()) {
        return;
    }

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("&Export"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        customButtonConnection = connect(wizard(), &QWizard::customButtonClicked, this, &PageStereoResult::exportCalibration);
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        QObject::disconnect(customButtonConnection);
    }
}


void PageStereoResult::exportCalibration ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export calibration to file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        QString ext = QFileInfo(fileName).completeSuffix();
        if (ext.isEmpty()) {
            ext = "yml";
            fileName += "." + ext;
        }

        try {
            QString fieldPrefix = "Stereo";

            // Export
            Pipeline::Rectification::exportStereoCalibration(fileName,
                field(fieldPrefix + "CameraMatrix1").value<cv::Mat>(),
                field(fieldPrefix + "DistCoeffs1").value<cv::Mat>(),
                field(fieldPrefix + "CameraMatrix2").value<cv::Mat>(),
                field(fieldPrefix + "DistCoeffs2").value<cv::Mat>(),
                field(fieldPrefix + "R").value<cv::Mat>(),
                field(fieldPrefix + "T").value<cv::Mat>(),
                field(fieldPrefix + "ImageSize").value<cv::Size>()
            );
        } catch (QString &e) {
            QMessageBox::warning(this, "Error", "Failed to export calibration: " + e);
        }
    }
}


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL
