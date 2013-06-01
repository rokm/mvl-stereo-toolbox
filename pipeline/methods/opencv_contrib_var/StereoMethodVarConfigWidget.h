/*
 * OpenCV Variational Matching: config widget
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

#ifndef STEREO_METHOD_VAR_CONFIG_WIDGET_H
#define STEREO_METHOD_VAR_CONFIG_WIDGET_H

#include <QtGui>


class StereoMethodVar;

class StereoMethodVarConfigWidget : public QWidget
{
    Q_OBJECT
    
public:
    StereoMethodVarConfigWidget (StereoMethodVar *, QWidget * = 0);
    virtual ~StereoMethodVarConfigWidget ();

protected slots:
    void presetChanged (int);

    void penalizationChanged (int);
    void cycleChanged (int);
    void flagsChanged ();

    void updateParameters ();

protected:
    StereoMethodVar *method;

    QComboBox *comboBoxPreset;

    QSpinBox *spinBoxLevels;
    QDoubleSpinBox *spinBoxPyrScale;
    QSpinBox *spinBoxNumIterations;
    QSpinBox *spinBoxMinDisparity;
    QSpinBox *spinBoxMaxDisparity;
    QSpinBox *spinBoxPolyN;
    QDoubleSpinBox *spinBoxPolySigma;
    QDoubleSpinBox *spinBoxFi;
    QDoubleSpinBox *spinBoxLambda;

    QComboBox *comboBoxPenalization;

    QComboBox *comboBoxCycle;
    
    QCheckBox *checkBoxUseInitialDisparity;
    QCheckBox *checkBoxUseEqualizeHist;
    QCheckBox *checkBoxUseSmartId;
    QCheckBox *checkBoxUseAutoParams;
    QCheckBox *checkBoxUseMedianFiltering;
};

#endif
