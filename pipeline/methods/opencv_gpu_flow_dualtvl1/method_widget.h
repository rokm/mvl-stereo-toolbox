/*
 * OpenCV GPU Dense Optical Flow - Dual TVL1: config widget
 * Copyright (C) 2013 Rok Mandeljc
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

#ifndef STEREO_METHOD_FLOW_DUALTVL1_GPU_CONFIG_WIDGET_H
#define STEREO_METHOD_FLOW_DUALTVL1_GPU_CONFIG_WIDGET_H

#include <QtWidgets>


namespace StereoMethodFlowDualTVL1Gpu {
    
class Method;

class MethodWidget : public QWidget
{
    Q_OBJECT
    
public:
    MethodWidget (Method *, QWidget * = 0);
    virtual ~MethodWidget ();

protected slots:
    void updateParameters ();

protected:
    Method *method;

    QCheckBox *checkBoxReverseImages;

    QDoubleSpinBox *spinBoxTau;
    QDoubleSpinBox *spinBoxLambda;
    QDoubleSpinBox *spinBoxTheta;
    QSpinBox *spinBoxNumberOfScales;
    QSpinBox *spinBoxNumberOfWarps;
    QDoubleSpinBox *spinBoxEpsilon;
    QSpinBox *spinBoxNumberOfIterations;
};

}

#endif
