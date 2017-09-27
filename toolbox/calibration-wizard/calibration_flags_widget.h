/*
 * MVL Stereo Toolbox: calibration wizard: calibration flags widget
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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__CALIBRATION_FLAGS_WIDGET_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__CALIBRATION_FLAGS_WIDGET_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                     Calibration flags widget                      *
// *********************************************************************
class CalibrationFlagsWidget : public QGroupBox
{
    Q_OBJECT

public:
    CalibrationFlagsWidget (const QString &title = "Flags", QWidget *parent = Q_NULLPTR);
    virtual ~CalibrationFlagsWidget ();

    virtual int getFlags () const;
    virtual void setFlags (int flags);

protected:
    QCheckBox *checkBoxUseIntrinsicGuess;
    QCheckBox *checkBoxFixPrincipalPoint;
    QCheckBox *checkBoxFixAspectRatio;
    QCheckBox *checkBoxZeroTangentDist;
    QCheckBox *checkBoxRationalModel;
    QCheckBox *checkBoxFixK1;
    QCheckBox *checkBoxFixK2;
    QCheckBox *checkBoxFixK3;
    QCheckBox *checkBoxFixK4;
    QCheckBox *checkBoxFixK5;
    QCheckBox *checkBoxFixK6;
};


// *********************************************************************
// *                  Stereo calibration flags widget                  *
// *********************************************************************
class StereoCalibrationFlagsWidget : public CalibrationFlagsWidget
{
    Q_OBJECT

public:
    StereoCalibrationFlagsWidget (const QString &title = "Flags", QWidget *parent = Q_NULLPTR);
    virtual ~StereoCalibrationFlagsWidget ();

    virtual int getFlags () const override;
    virtual void setFlags (int flags) override;

protected:
    QCheckBox *checkBoxFixIntrinsic;
    QCheckBox *checkBoxFixFocalLength;
    QCheckBox *checkBoxSameFocalLength;
};


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL


#endif
