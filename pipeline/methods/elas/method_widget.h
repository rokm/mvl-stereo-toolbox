/*
 * Efficient LArge-scale Stereo: method widget
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__METHODS__ELAS__METHOD_WIDGET_H
#define MVL_STEREO_TOOLBOX__PIPELINE__METHODS__ELAS__METHOD_WIDGET_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodELAS {


class Method;

class MethodWidget : public QWidget
{
    Q_OBJECT

public:
    MethodWidget (Method *, QWidget * = 0);
    virtual ~MethodWidget ();

protected:
    void updateParameters ();

protected:
    Method *method;

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


} // StereoMethodELAS
} // Pipeline
} // StereoToolbox
} // MVL


#endif
