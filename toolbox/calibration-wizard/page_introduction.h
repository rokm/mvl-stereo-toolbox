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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__PAGE_INTRODUCTION_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__PAGE_INTRODUCTION_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


class PageIntroduction : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(QString calibrationType READ getCalibrationType);

public:
    PageIntroduction (QWidget *parent = Q_NULLPTR);
    virtual ~PageIntroduction ();

    virtual int nextId () const override;

    QString getCalibrationType () const;

protected:
    QRadioButton *radioButtonJointCalibration;
    QRadioButton *radioButtonDecoupledCalibration;
    QRadioButton *radioButtonSingleCalibration;
};


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL


#endif
