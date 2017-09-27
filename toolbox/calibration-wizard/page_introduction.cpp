/*
 * MVL Stereo Toolbox: calibration wizard: introduction page
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

#include "page_introduction.h"
#include "wizard.h"


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


PageIntroduction::PageIntroduction (QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Introduction");

    QLabel *label;
    QFrame *separator;

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("This wizard will guide you through stereo calibration process.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label);

    // Separator
    separator = new QFrame(this);
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(separator);

    // Calibration mode
    label = new QLabel("Please select desired calibration mode below. In <i>joint calibration</i>, you will be "
                       "asked for a single sequence of image pairs, from which both cameras' intrinsic parameters "
                       "(camera matrix and distortion coefficients) and stereo parameters will be estimated. In <i>decoupled "
                       "calibration</i>, you will be first asked for one sequence of calibration images for each camera, and their "
                       "intrinsic parameters will be estimated separately. Then, you will be asked for a sequence of image pairs "
                       "from which stereo parameters will be estimated.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label);

    radioButtonJointCalibration = new QRadioButton("&Joint calibration");
    radioButtonJointCalibration->setToolTip("Estimate all parameters together.");
    layout->addWidget(radioButtonJointCalibration);

    radioButtonDecoupledCalibration = new QRadioButton("&Decoupled calibration");
    radioButtonDecoupledCalibration->setToolTip("Separately estimate intrinsic parameters and stereo parameters.");
    layout->addWidget(radioButtonDecoupledCalibration);

    registerField("JointCalibration", radioButtonJointCalibration);

    radioButtonJointCalibration->setChecked(true);
}

PageIntroduction::~PageIntroduction ()
{
}

int PageIntroduction::nextId () const
{
    if (radioButtonJointCalibration->isChecked()) {
        return Wizard::PageId::StereoImagesId;
    } else {
        return Wizard::PageId::LeftCameraImagesId;
    }
}


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL
