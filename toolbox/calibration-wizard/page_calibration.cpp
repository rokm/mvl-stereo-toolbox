/*
 * MVL Stereo Toolbox: calibration wizard: calibration page
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

#include "page_calibration.h"
#include "wizard.h"
#include "calibration_flags_widget.h"
#include "camera_parameters_widget.h"

#include <QtConcurrent>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                      Calibration page: common                     *
// *********************************************************************
PageCalibration::PageCalibration (const QString &fieldPrefixString, QWidget *parent)
    : QWizardPage(parent),
      fieldPrefix(fieldPrefixString)
{
    setSubTitle("Calibration parameters");

    // Busy dialog
    dialogBusy = new QProgressDialog("Calibrating... please wait", QString(), 0, 0, this);
    dialogBusy->setModal(true);
    dialogBusy->reset(); // Workaround for Qt 5.5.0 bug (QTBUG-47042)

    // Worker thread
    calibrationComplete = false;
    connect(&calibrationWatcher, &QFutureWatcher<bool>::finished, this, &PageCalibration::calibrationFinished);

    QLabel *label;

    // Layout
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Please enter parameters for calibration method, and press \"Calibrate\" to perform calibration. Depending on "
                       "number of images and hardware, the operation might take some time.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label, 0, 0, 1, 2);

    // Camera parameters
    widgetCameraParameters = new CameraParametersWidget("Parameters", this);
    layout->addWidget(widgetCameraParameters, 1, 0, 1, 1);

    // Calibration Flags
    widgetCalibrationFlags = new CalibrationFlagsWidget("Flags", this);
    layout->addWidget(widgetCalibrationFlags, 1, 1, 1, 1);

    widgetCalibrationFlags->setFlags(cv::CALIB_RATIONAL_MODEL); // Default flags

    // Fields
    registerField(fieldPrefix + "CameraMatrix", this, "cameraMatrix");
    registerField(fieldPrefix + "DistCoeffs", this, "distCoeffs");

    // Needed for transfer between pages
    registerField(fieldPrefix + "CalibrationFlags", this, "calibrationFlags");
}

PageCalibration::~PageCalibration ()
{
}


cv::Mat PageCalibration::getCameraMatrix () const
{
    return cameraMatrix;
}

void PageCalibration::setCameraMatrix (const cv::Mat &camera)
{
    cameraMatrix = camera;
}


cv::Mat PageCalibration::getDistCoeffs () const
{
    return distCoeffs;
}

void PageCalibration::setDistCoeffs (const cv::Mat &coeffs)
{
    distCoeffs = coeffs;
}


void PageCalibration::setCalibrationFlags (int flags)
{
    widgetCalibrationFlags->setFlags(flags);
}

int PageCalibration::getCalibrationFlags () const
{
    return widgetCalibrationFlags->getFlags();
}


void PageCalibration::setVisible (bool visible)
{
    QWizardPage::setVisible(visible);

    // On Windows, this function gets called without wizard being set...
    if (!wizard()) {
        return;
    }

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("&Calibrate"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        customButtonConnection = connect(wizard(), &QWizard::customButtonClicked, this, &PageCalibration::calibrationBegin);
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        QObject::disconnect(customButtonConnection);
    }
}

void PageCalibration::initializePage ()
{
    // Initialize principal point in the camera parameters widget
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();
    widgetCameraParameters->setPrincipalPointX(imageSize.width/2);
    widgetCameraParameters->setPrincipalPointY(imageSize.height/2);

    calibrationComplete = false;
}

bool PageCalibration::isComplete () const
{
    // Ready when calibration has been computed
    return calibrationComplete;
}


void PageCalibration::calibrationBegin ()
{
    // Show busy dialog
    dialogBusy->show();

    // Start calibration function
    QFuture<bool> future = QtConcurrent::run(this, &PageCalibration::calibrationFunction);
    calibrationWatcher.setFuture(future);
}

void PageCalibration::calibrationFinished ()
{
    dialogBusy->hide();

    // Read result
    calibrationComplete = calibrationWatcher.result();

    // Display dialog
    if (calibrationComplete) {
        QMessageBox::information(this, "Calibration finished", QString("Calibration finished!\nRMSE: %1").arg(calibrationRMSE));
    }

    // We might be ready
    emit completeChanged();
}

bool PageCalibration::calibrationFunction ()
{
    // Reset calibration
    calibrationComplete = false;

    std::vector<std::vector<cv::Point2f> > imagePoints = field(fieldPrefix + "PatternImagePoints").value< std::vector<std::vector<cv::Point2f> > >();
    std::vector<std::vector<cv::Point3f> > worldPoints = field(fieldPrefix + "PatternWorldPoints").value< std::vector<std::vector<cv::Point3f> > >();
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();

    // Get values from the config widgets
    int flags = widgetCalibrationFlags->getFlags();
    cameraMatrix = widgetCameraParameters->getCameraMatrix();
    distCoeffs = cv::Mat(widgetCameraParameters->getDistCoeffs()).clone();

    try {
        calibrationRMSE = cv::calibrateCamera(worldPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, cv::noArray(), cv::noArray(), flags);
    } catch (cv::Exception &e) {
        emit error("Calibration failed: " + QString::fromStdString(e.what()));
        return false;
    }

    return true;
}


// *********************************************************************
// *                   Calibration page: single camera                 *
// *********************************************************************
PageSingleCameraCalibration::PageSingleCameraCalibration (QWidget *parent)
    : PageCalibration("SingleCamera", parent)
{
    setTitle("Single camera calibration");
}

PageSingleCameraCalibration::~PageSingleCameraCalibration ()
{
}

int PageSingleCameraCalibration::nextId () const
{
    return Wizard::PageId::SingleCameraResultId;
}


// *********************************************************************
// *                    Calibration page: left camera                  *
// *********************************************************************
PageLeftCameraCalibration::PageLeftCameraCalibration (QWidget *parent)
    : PageCalibration("LeftCamera", parent)
{
    setTitle("Left camera calibration");
}

PageLeftCameraCalibration::~PageLeftCameraCalibration ()
{
}

int PageLeftCameraCalibration::nextId () const
{
    return Wizard::PageId::LeftCameraResultId;
}


// *********************************************************************
// *                   Calibration page: right camera                  *
// *********************************************************************
PageRightCameraCalibration::PageRightCameraCalibration (QWidget *parent)
    : PageCalibration("RightCamera", parent)
{
    setTitle("Right camera calibration");
}

PageRightCameraCalibration::~PageRightCameraCalibration ()
{
}

int PageRightCameraCalibration::nextId () const
{
    return Wizard::PageId::RightCameraResultId;
}

void PageRightCameraCalibration::initializePage ()
{
    PageCalibration::initializePage();

    // Copy calibration flags setting from left camera
    QString leftCameraPrefix = "LeftCamera";
    setField(fieldPrefix + "CalibrationFlags", field(leftCameraPrefix + "CalibrationFlags"));
}


// *********************************************************************
// *                      Calibration page: stereo                     *
// *********************************************************************
PageStereoCalibration::PageStereoCalibration (QWidget *parent)
    : QWizardPage(parent),
      fieldPrefix("Stereo")
{
    setSubTitle("Calibration parameters");

    // Busy dialog
    dialogBusy = new QProgressDialog("Calibrating... please wait", QString(), 0, 0, this);
    dialogBusy->setModal(true);
    dialogBusy->reset(); // Workaround for Qt 5.5.0 bug (QTBUG-47042)

    // Worker thread
    calibrationComplete = false;
    connect(&calibrationWatcher, &QFutureWatcher<bool>::finished, this, &PageStereoCalibration::calibrationFinished);

    QLabel *label;

    // Layout
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Please enter parameters for calibration method, and press \"Calibrate\" to perform calibration. Depending on "
                       "number of images and hardware, the operation might take some time, during which the GUI will appear to be frozen.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label, 0, 0, 1, 3);

    // Left camera parameters
    widgetLeftCameraParameters = new CameraParametersWidget("Left camera", this);
    layout->addWidget(widgetLeftCameraParameters, 1, 0, 1, 1);

    // Right camera parameters
    widgetRightCameraParameters = new CameraParametersWidget("Right camera", this);
    layout->addWidget(widgetRightCameraParameters, 1, 1, 1, 1);

    // Calibration flags
    widgetCalibrationFlags = new StereoCalibrationFlagsWidget("Flags", this);
    layout->addWidget(widgetCalibrationFlags, 1, 2, 1, 1);

    // Fields
    registerField(fieldPrefix + "CameraMatrix1", this, "cameraMatrix1");
    registerField(fieldPrefix + "DistCoeffs1", this, "distCoeffs1");
    registerField(fieldPrefix + "CameraMatrix2", this, "cameraMatrix2");
    registerField(fieldPrefix + "DistCoeffs2", this, "distCoeffs2");
    registerField(fieldPrefix + "R", this, "R");
    registerField(fieldPrefix + "T", this, "T");
}

PageStereoCalibration::~PageStereoCalibration ()
{
}

const cv::Mat &PageStereoCalibration::getCameraMatrix1 () const
{
    return cameraMatrix1;
}

const cv::Mat &PageStereoCalibration::getDistCoeffs1 () const
{
    return distCoeffs1;
}

const cv::Mat &PageStereoCalibration::getCameraMatrix2 () const
{
    return cameraMatrix2;
}

const cv::Mat &PageStereoCalibration::getDistCoeffs2 () const
{
    return distCoeffs2;
}

const cv::Mat &PageStereoCalibration::getR () const
{
    return R;
}

const cv::Mat &PageStereoCalibration::getT () const
{
    return T;
}

const cv::Size &PageStereoCalibration::getImageSize () const
{
    return imageSize;
}


void PageStereoCalibration::setVisible (bool visible)
{
    QWizardPage::setVisible(visible);

    // On Windows, this function gets called without wizard being set...
    if (!wizard()) {
        return;
    }

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("&Calibrate"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        customButtonConnection = connect(wizard(), &QWizard::customButtonClicked, this, &PageStereoCalibration::calibrationBegin);
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        QObject::disconnect(customButtonConnection);
    }
}


void PageStereoCalibration::initializePage ()
{
    if (field("CalibrationType").toString() == "JointCalibration") {
        // Initialize principal point in the camera parameters widget
        cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();
        widgetLeftCameraParameters->setPrincipalPointX(imageSize.width/2);
        widgetLeftCameraParameters->setPrincipalPointY(imageSize.height/2);

        widgetRightCameraParameters->setPrincipalPointX(imageSize.width/2);
        widgetRightCameraParameters->setPrincipalPointY(imageSize.height/2);

        widgetCalibrationFlags->setFlags(cv::CALIB_RATIONAL_MODEL); // Default flags
    } else {
        // Disjoint calibration; load camera matrices and distortion
        // coefficients, and set CALIB_FIX_INTRINSIC by default
        cv::Mat M1 = field("LeftCameraCameraMatrix").value<cv::Mat>();
        cv::Mat D1 = field("LeftCameraDistCoeffs").value<cv::Mat>();
        widgetLeftCameraParameters->setCameraMatrix(M1, D1);

        cv::Mat M2 = field("RightCameraCameraMatrix").value<cv::Mat>();
        cv::Mat D2 = field("RightCameraDistCoeffs").value<cv::Mat>();
        widgetRightCameraParameters->setCameraMatrix(M2, D2);

        widgetCalibrationFlags->setFlags(cv::CALIB_RATIONAL_MODEL | cv::CALIB_FIX_INTRINSIC); // Default flags
    }

    calibrationComplete = false;
}

bool PageStereoCalibration::isComplete () const
{
    // Ready when calibration has been computed
    return calibrationComplete;
}


void PageStereoCalibration::calibrationBegin ()
{
    // Show busy dialog
    dialogBusy->show();

    // Start calibration function
    QFuture<bool> future = QtConcurrent::run(this, &PageStereoCalibration::calibrationFunction);
    calibrationWatcher.setFuture(future);
}

void PageStereoCalibration::calibrationFinished ()
{
    dialogBusy->hide();

    // Read result
    calibrationComplete = calibrationWatcher.result();

    // Display dialog
    if (calibrationComplete) {
        QMessageBox::information(this, "Calibration finished", QString("Calibration finished!\nRMSE: %1\nAvg. reprojection error: %2").arg(calibrationRMSE).arg(averageReprojectionError));
    }

    // We might be ready
    emit completeChanged();
}


bool PageStereoCalibration::calibrationFunction ()
{
    // Reset calibration
    calibrationComplete = false;

    std::vector<std::vector<cv::Point2f> > imagePoints = field(fieldPrefix + "PatternImagePoints").value< std::vector<std::vector<cv::Point2f> > >();
    std::vector<std::vector<cv::Point3f> > objectPoints = field(fieldPrefix + "PatternWorldPoints").value< std::vector<std::vector<cv::Point3f> > >();
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();

    // Perform calibration here - we use OpenCV directly and bypass
    // the pipeline's stereo calibration function, so that calibration
    // can be cancelled at any point without affecting the pipeline...

    // We need to split image points in two
    std::vector<std::vector<cv::Point2f> > imagePoints1;
    std::vector<std::vector<cv::Point2f> > imagePoints2;

    for (unsigned int i = 0; i < objectPoints.size(); i++) {
        imagePoints1.push_back(imagePoints[2*i]);
        imagePoints2.push_back(imagePoints[2*i+1]);
    }

    // Get values from the config widgets
    int flags = widgetCalibrationFlags->getFlags();
    cameraMatrix1 = widgetLeftCameraParameters->getCameraMatrix();
    distCoeffs1 = cv::Mat(widgetLeftCameraParameters->getDistCoeffs()).clone();
    cameraMatrix2 = widgetRightCameraParameters->getCameraMatrix();
    distCoeffs2 = cv::Mat(widgetRightCameraParameters->getDistCoeffs()).clone();

    cv::Mat E, F;

    try {
        calibrationRMSE = cv::stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
                                              cameraMatrix1, distCoeffs1,
                                              cameraMatrix2, distCoeffs2,
                                              imageSize, R, T, E, F,
                                              flags,
                                              cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-6));
    } catch (cv::Exception &e) {
        emit error("Calibration failed: " + QString::fromStdString(e.what()));
        return false;
    }

    // Compute average reprojection error
    averageReprojectionError = 0;
    int num_points = 0;
    std::vector<cv::Vec3f> lines1, lines2;

    for (unsigned int i = 0; i < objectPoints.size(); i++) {
        int npt = imagePoints1[i].size();
        cv::Mat imgpt1, imgpt2;

        // First image
        imgpt1 = cv::Mat(imagePoints1[i]);
        cv::undistortPoints(imgpt1, imgpt1, cameraMatrix1, distCoeffs1, cv::Mat(), cameraMatrix1);
        cv::computeCorrespondEpilines(imgpt1, 1, F, lines1);

        // Second image
        imgpt2 = cv::Mat(imagePoints2[i]);
        cv::undistortPoints(imgpt2, imgpt2, cameraMatrix2, distCoeffs2, cv::Mat(), cameraMatrix2);
        cv::computeCorrespondEpilines(imgpt2, 2, F, lines2);

        // Compute error
        for (int j = 0; j < npt; j++) {
            double errij = std::abs(imagePoints1[i][j].x*lines2[j][0] +
                                    imagePoints1[i][j].y*lines2[j][1] + lines2[j][2]) +
                           std::abs(imagePoints2[i][j].x*lines1[j][0] +
                                    imagePoints2[i][j].y*lines1[j][1] + lines1[j][2]);
            averageReprojectionError += errij;
        }

        num_points += npt;
    }
    averageReprojectionError /= num_points;

    return true;
}



} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL
