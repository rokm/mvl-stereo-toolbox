/*
 * MVL Stereo Toolbox: calibration wizard: camera parameters widget
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

#include "camera_parameters_widget.h"

#include <opencv2/calib3d.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                     Camera parameters widget                      *
// *********************************************************************
CameraParametersWidget::CameraParametersWidget (const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    QFormLayout *groupBoxLayout = new QFormLayout(this);
    groupBoxLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    QLabel *label;
    QDoubleSpinBox *spinBoxD;

    // Fx
    label = new QLabel("fx", this);
    label->setToolTip("Focal length (horizontal) in pixel units");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(2);
    spinBoxD->setSuffix(" px");
    spinBoxFx = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // Fy
    label = new QLabel("fy", this);
    label->setToolTip("Focal length (vertical) in pixel units");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(2);
    spinBoxD->setSuffix(" px");
    spinBoxFy = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // Cx
    label = new QLabel("cx", this);
    label->setToolTip("Horizontal coordinate of principal point");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(2);
    spinBoxD->setSuffix(" px");
    spinBoxCx = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // Cy
    label = new QLabel("cy", this);
    label->setToolTip("Vertical coordinate of principal point");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(2);
    spinBoxD->setSuffix(" px");
    spinBoxCy = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // K1
    label = new QLabel("k1", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK1 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // K2
    label = new QLabel("k2", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK2 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // P1
    label = new QLabel("p1", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxP1 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);


    // P2
    label = new QLabel("p2", this);
    label->setToolTip("Tangential distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxP2 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);


    // K3
    label = new QLabel("k3", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK3 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // K4
    label = new QLabel("k4", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK4 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // K5
    label = new QLabel("k5", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK5 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // K6
    label = new QLabel("k6", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK6 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);
}

CameraParametersWidget::~CameraParametersWidget ()
{
}

double CameraParametersWidget::getFocalLengthX () const
{
    return spinBoxFx->value();
}

void CameraParametersWidget::setFocalLengthX (double value)
{
    spinBoxFx->setValue(value);
}

double CameraParametersWidget::getFocalLengthY () const
{
    return spinBoxFy->value();
}

void CameraParametersWidget::setFocalLengthY (double value)
{
    spinBoxFy->setValue(value);
}


double CameraParametersWidget::getPrincipalPointX () const
{
    return spinBoxCx->value();
}

void CameraParametersWidget::setPrincipalPointX (double value)
{
    spinBoxCx->setValue(value);
}


double CameraParametersWidget::getPrincipalPointY () const
{
    return spinBoxCy->value();
}

void CameraParametersWidget::setPrincipalPointY (double value)
{
    spinBoxCy->setValue(value);
}


double CameraParametersWidget::getDistortionK1 () const
{
    return spinBoxK1->value();
}

void CameraParametersWidget::setDistortionK1 (double value)
{
    spinBoxK1->setValue(value);
}


double CameraParametersWidget::getDistortionK2 () const
{
    return spinBoxK2->value();
}

void CameraParametersWidget::setDistortionK2 (double value)
{
    spinBoxK2->setValue(value);
}


double CameraParametersWidget::getDistortionK3 () const
{
    return spinBoxK3->value();
}

void CameraParametersWidget::setDistortionK3 (double value)
{
    spinBoxK3->setValue(value);
}


double CameraParametersWidget::getDistortionP1 () const
{
    return spinBoxP1->value();
}

void CameraParametersWidget::setDistortionP1 (double value)
{
    spinBoxP1->setValue(value);
}


double CameraParametersWidget::getDistortionP2 () const
{
    return spinBoxP2->value();
}

void CameraParametersWidget::setDistortionP2 (double value)
{
    spinBoxP2->setValue(value);
}


double CameraParametersWidget::getDistortionK4 () const
{
    return spinBoxK4->value();
}

void CameraParametersWidget::setDistortionK4 (double value)
{
    spinBoxK4->setValue(value);
}


double CameraParametersWidget::getDistortionK5 () const
{
    return spinBoxK5->value();
}

void CameraParametersWidget::setDistortionK5 (double value)
{
    spinBoxK5->setValue(value);
}


double CameraParametersWidget::getDistortionK6 () const
{
    return spinBoxK6->value();
}

void CameraParametersWidget::setDistortionK6 (double value)
{
    spinBoxK6->setValue(value);
}

void CameraParametersWidget::setCameraMatrix (const cv::Mat &cameraMatrix, const std::vector<double> &distCoeffs)
{
    // Read values of interest from camera matrix
    setFocalLengthX(cameraMatrix.at<double>(0, 0));
    setFocalLengthY(cameraMatrix.at<double>(1, 1));
    setPrincipalPointX(cameraMatrix.at<double>(0, 2));
    setPrincipalPointY(cameraMatrix.at<double>(1, 2));

    // K1, K2, P1 and P2 are always present
    setDistortionK1(distCoeffs[0]);
    setDistortionK2(distCoeffs[1]);
    setDistortionP1(distCoeffs[2]);
    setDistortionP2(distCoeffs[3]);

    // K3 is optional
    if (distCoeffs.size() > 4) {
        setDistortionK3(distCoeffs[4]);
    } else {
        setDistortionK3(0);
    }

    // K4, K5 and K6 are present only for rational models
    if (distCoeffs.size() > 5) {
        setDistortionK4(distCoeffs[5]);
        setDistortionK5(distCoeffs[6]);
        setDistortionK6(distCoeffs[7]);
    } else {
        setDistortionK4(0);
        setDistortionK5(0);
        setDistortionK6(0);
    }
}

cv::Mat CameraParametersWidget::getCameraMatrix () const
{
    cv::Mat M = cv::Mat::eye(3, 3, CV_64F);

    M.at<double>(0, 0) = getFocalLengthX();
    M.at<double>(1, 1) = getFocalLengthX();
    M.at<double>(0, 2) = getPrincipalPointX();
    M.at<double>(1, 2) = getPrincipalPointY();

    return M;
}

std::vector<double> CameraParametersWidget::getDistCoeffs () const
{
    std::vector<double> D(8);

    D[0] = getDistortionK1();
    D[1] = getDistortionK2();
    D[2] = getDistortionP1();
    D[3] = getDistortionP2();
    D[4] = getDistortionK3();
    D[5] = getDistortionK4();
    D[6] = getDistortionK5();
    D[7] = getDistortionK6();

    return D;
}


void CameraParametersWidget::setDisplayMode (bool displayMode)
{
    if (displayMode) {
        spinBoxFx->setEnabled(false);
        spinBoxFy->setEnabled(false);
        spinBoxCx->setEnabled(false);
        spinBoxCy->setEnabled(false);
        spinBoxK1->setEnabled(false);
        spinBoxK2->setEnabled(false);
        spinBoxP1->setEnabled(false);
        spinBoxP2->setEnabled(false);
        spinBoxK3->setEnabled(false);
        spinBoxK4->setEnabled(false);
        spinBoxK5->setEnabled(false);
        spinBoxK6->setEnabled(false);
    } else {
        spinBoxFx->setEnabled(true);
        spinBoxFy->setEnabled(true);
        spinBoxCx->setEnabled(true);
        spinBoxCy->setEnabled(true);
        spinBoxK1->setEnabled(true);
        spinBoxK2->setEnabled(true);
        spinBoxP1->setEnabled(true);
        spinBoxP2->setEnabled(true);
        spinBoxK3->setEnabled(true);
        spinBoxK4->setEnabled(true);
        spinBoxK5->setEnabled(true);
        spinBoxK6->setEnabled(true);
    }
}


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL
