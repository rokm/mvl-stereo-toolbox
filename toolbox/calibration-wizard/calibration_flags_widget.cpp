/*
 * MVL Stereo Toolbox: calibration wizard: calibration flags widget
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

#include "calibration_flags_widget.h"

#include <opencv2/calib3d.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                     Calibration flags widget                      *
// *********************************************************************
CalibrationFlagsWidget::CalibrationFlagsWidget (const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    QFormLayout *groupBoxLayout = new QFormLayout(this);
    QCheckBox *checkBox;

    // CALIB_USE_INTRINSIC_GUESS
    checkBox = new QCheckBox("CALIB_USE_INTRINSIC_GUESS", this);
    checkBox->setToolTip("The input values for fx, fy, cx and cy are considered an initial guess \n"
                         "and are optimized further. Otherwise, (cx, cy) is initially set to the \n"
                         "image center and focal distances are computed in a least-squares fashion.");
    checkBoxUseIntrinsicGuess = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_PRINCIPAL_POINT
    checkBox = new QCheckBox("CALIB_FIX_PRINCIPAL_POINT", this);
    checkBox->setToolTip("The principal point is not changed during the global optimization.\n"
                         "It stays at the center or, if CALIB_USE_INTRINSIC_GUESS is set, at \n"
                         "the specified location.");
    checkBoxFixPrincipalPoint = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_ASPECT_RATIO
    checkBox = new QCheckBox("CALIB_FIX_ASPECT_RATIO", this);
    checkBox->setToolTip("The function considers only fy as a free parameter. The ratio fx/fy\n"
                         "stays the same as defined by the input values. If CALIB_USE_INTRINSIC_GUESS \n"
                         "is not set, the actual input values of fx and fy are ignored, and only their \n"
                         "ratio is computed and used further.");
    checkBoxFixAspectRatio = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_ZERO_TANGENT_DIST
    checkBox = new QCheckBox("CALIB_ZERO_TANGENT_DIST", this);
    checkBox->setToolTip("Tangential distortion coefficients (p1, p2) are set to zeros and stay zero.");
    checkBoxZeroTangentDist = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_RATIONAL_MODEL
    checkBox = new QCheckBox("CALIB_RATIONAL_MODEL", this);
    checkBox->setToolTip("Coefficients k4, k5, and k6 are enabled, totally amounting to 8 distortion coefficients.\n"
                         "If the flag is not set, the function computes and returns only 5 distortion coefficients.");
    checkBoxRationalModel = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K1
    checkBox = new QCheckBox("CALIB_FIX_K1", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK1 = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K2
    checkBox = new QCheckBox("CALIB_FIX_K2", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK2 = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K3
    checkBox = new QCheckBox("CALIB_FIX_K3", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK3 = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K4
    checkBox = new QCheckBox("CALIB_FIX_K4", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK4 = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K5
    checkBox = new QCheckBox("CALIB_FIX_K5", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK5 = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K6
    checkBox = new QCheckBox("CALIB_FIX_K6", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK6 = checkBox;
    groupBoxLayout->addRow(checkBox);
}

CalibrationFlagsWidget::~CalibrationFlagsWidget ()
{
}


int CalibrationFlagsWidget::getFlags () const
{
    int flags = 0;

    flags |= (checkBoxUseIntrinsicGuess->checkState() == Qt::Checked) * cv::CALIB_USE_INTRINSIC_GUESS;
    flags |= (checkBoxFixPrincipalPoint->checkState() == Qt::Checked) * cv::CALIB_FIX_PRINCIPAL_POINT;
    flags |= (checkBoxFixAspectRatio->checkState() == Qt::Checked) * cv::CALIB_FIX_ASPECT_RATIO;
    flags |= (checkBoxZeroTangentDist->checkState() == Qt::Checked) * cv::CALIB_ZERO_TANGENT_DIST;
    flags |= (checkBoxRationalModel->checkState() == Qt::Checked) * cv::CALIB_RATIONAL_MODEL;
    flags |= (checkBoxFixK1->checkState() == Qt::Checked) * cv::CALIB_FIX_K1;
    flags |= (checkBoxFixK2->checkState() == Qt::Checked) * cv::CALIB_FIX_K2;
    flags |= (checkBoxFixK3->checkState() == Qt::Checked) * cv::CALIB_FIX_K3;
    flags |= (checkBoxFixK4->checkState() == Qt::Checked) * cv::CALIB_FIX_K4;
    flags |= (checkBoxFixK5->checkState() == Qt::Checked) * cv::CALIB_FIX_K5;
    flags |= (checkBoxFixK6->checkState() == Qt::Checked) * cv::CALIB_FIX_K6;

    return flags;
}

void CalibrationFlagsWidget::setFlags (int flags)
{
    checkBoxUseIntrinsicGuess->setChecked(flags & cv::CALIB_USE_INTRINSIC_GUESS);
    checkBoxFixPrincipalPoint->setChecked(flags & cv::CALIB_FIX_PRINCIPAL_POINT);
    checkBoxFixAspectRatio->setChecked(flags & cv::CALIB_FIX_ASPECT_RATIO);
    checkBoxZeroTangentDist->setChecked(flags & cv::CALIB_ZERO_TANGENT_DIST);
    checkBoxRationalModel->setChecked(flags & cv::CALIB_RATIONAL_MODEL);
    checkBoxFixK1->setChecked(flags & cv::CALIB_FIX_K1);
    checkBoxFixK2->setChecked(flags & cv::CALIB_FIX_K2);
    checkBoxFixK3->setChecked(flags & cv::CALIB_FIX_K3);
    checkBoxFixK4->setChecked(flags & cv::CALIB_FIX_K4);
    checkBoxFixK5->setChecked(flags & cv::CALIB_FIX_K5);
    checkBoxFixK6->setChecked(flags & cv::CALIB_FIX_K6);
}


// *********************************************************************
// *                  Stereo calibration flags widget                  *
// *********************************************************************
StereoCalibrationFlagsWidget::StereoCalibrationFlagsWidget (const QString &title, QWidget *parent)
    : CalibrationFlagsWidget(title, parent)
{
    QFormLayout *groupBoxLayout = qobject_cast<QFormLayout *>(layout());
    QCheckBox *checkBox;

    // CALIB_FIX_INTRINSIC
    checkBox = new QCheckBox("CALIB_FIX_INTRINSIC", this);
    checkBox->setToolTip("Fix camera matrices and distortion coefficients, so that only inter-camera rotation and \n"
                         "translation matrices (R and T) are estimated.");
    checkBoxFixIntrinsic = checkBox;
    groupBoxLayout->insertRow(0, checkBox);

    // CALIB_FIX_FOCAL_LENGTH
    checkBox = new QCheckBox("CALIB_FIX_FOCAL_LENGTH", this);
    checkBox->setToolTip("Fix values of fx and fy on both cameras.");
    checkBoxFixFocalLength = checkBox;
    groupBoxLayout->insertRow(3, checkBox);

    // CALIB_SAME_FOCAL_LENGTH
    checkBox = new QCheckBox("CALIB_SAME_FOCAL_LENGTH", this);
    checkBox->setToolTip("Enforce same focal length on both cameras.");
    checkBoxSameFocalLength = checkBox;
    groupBoxLayout->insertRow(6, checkBox);
}

StereoCalibrationFlagsWidget::~StereoCalibrationFlagsWidget ()
{
}


int StereoCalibrationFlagsWidget::getFlags () const
{
    int flags = CalibrationFlagsWidget::getFlags();

    flags |= (checkBoxFixIntrinsic->checkState() == Qt::Checked) * cv::CALIB_FIX_INTRINSIC;
    flags |= (checkBoxFixFocalLength->checkState() == Qt::Checked) * cv::CALIB_FIX_FOCAL_LENGTH;
    flags |= (checkBoxSameFocalLength->checkState() == Qt::Checked) * cv::CALIB_SAME_FOCAL_LENGTH;

    return flags;
}

void StereoCalibrationFlagsWidget::setFlags (int flags)
{
    CalibrationFlagsWidget::setFlags(flags);

    checkBoxFixIntrinsic->setChecked(flags & cv::CALIB_FIX_INTRINSIC);
    checkBoxFixFocalLength->setChecked(flags & cv::CALIB_FIX_FOCAL_LENGTH);
    checkBoxSameFocalLength->setChecked(flags & cv::CALIB_SAME_FOCAL_LENGTH);
}


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL
