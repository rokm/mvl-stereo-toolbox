/*
 * Efficient LArge-scale Stereo: config widget
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

#ifndef STEREO_METHOD_ELAS_CONFIG_WIDGET_H
#define STEREO_METHOD_ELAS_CONFIG_WIDGET_H

#include <QtGui>


class StereoMethodELAS;

class StereoMethodELASConfigWidget : public QWidget
{
    Q_OBJECT
    
public:
    StereoMethodELASConfigWidget (StereoMethodELAS *, QWidget * = 0);
    virtual ~StereoMethodELASConfigWidget ();

protected slots:
    void presetChanged (int);

    void addCornersChanged (int);
    void medianFilterChanged (int);
    void adaptiveMeanFilterChanged (int);
    void postProcessOnlyLeftChanged (int);
    void subsamplingChanged (int);
    void returnLeftChanged (int);
    
    void updateParameters ();

protected:
    StereoMethodELAS *method;

    QComboBox *comboBoxPreset;

    QSpinBox *spinBoxMinDisparity;
    QSpinBox *spinBoxMaxDisparity;

    QDoubleSpinBox *spinBoxSupportThreshold;
    QSpinBox *spinBoxSupportTexture;
    QSpinBox *spinBoxCandidateStepSize;
    QSpinBox *spinBoxInconsistentWindowSize;
    QSpinBox *spinBoxInconsistentThreshold;
    QSpinBox *spinBoxInconsistentMinSupport;

    QCheckBox *checkBoxAddCorners;
    QSpinBox *spinBoxGridSize;
    
    QDoubleSpinBox *spinBoxBeta;
    QDoubleSpinBox *spinBoxGamma;
    QDoubleSpinBox *spinBoxSigma;
    QDoubleSpinBox *spinBoxSigmaRadius;

    QSpinBox *spinBoxMatchTexture;
    QSpinBox *spinBoxLRThreshold;

    QDoubleSpinBox *spinBoxSpeckleSimThreshold;
    QSpinBox *spinBoxSpeckleSize;
    QSpinBox *spinBoxInterpolationGapWidth;

    QCheckBox *checkBoxFilterMedian;
    QCheckBox *checkBoxFilterAdaptiveMean;
    QCheckBox *checkBoxPostProcessOnlyLeft;
    QCheckBox *checkBoxSubsampling;

    QCheckBox *checkBoxReturnLeft;
};

#endif
