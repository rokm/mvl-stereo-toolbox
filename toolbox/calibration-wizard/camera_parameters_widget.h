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

#ifndef MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__CAMERA_PARAMETERS_WIDGET_H
#define MVL_STEREO_TOOLBOX__TOOLBOX__CALIBRATION_WIZARD__CAMERA_PARAMETERS_WIDGET_H

#include <QtWidgets>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


class CameraParametersWidget : public QGroupBox
{
    Q_OBJECT

public:
    CameraParametersWidget (const QString &title = "Parameters", QWidget *parent = Q_NULLPTR);
    virtual ~CameraParametersWidget ();

    double getFocalLengthX () const;
    void setFocalLengthX (double value);

    double getFocalLengthY () const;
    void setFocalLengthY (double value);

    double getPrincipalPointX () const;
    void setPrincipalPointX (double value);

    double getPrincipalPointY () const;
    void setPrincipalPointY (double value);

    double getDistortionK1 () const;
    void setDistortionK1 (double value);

    double getDistortionK2 () const;
    void setDistortionK2 (double value);

    double getDistortionK3 () const;
    void setDistortionK3 (double value);

    double getDistortionP1 () const;
    void setDistortionP1 (double value);

    double getDistortionP2 () const;
    void setDistortionP2 (double value);

    double getDistortionK4 () const;
    void setDistortionK4 (double value);

    double getDistortionK5 () const;
    void setDistortionK5 (double value);

    double getDistortionK6 () const;
    void setDistortionK6 (double value);

    void setCameraMatrix (const cv::Mat &cameraMatrix, const std::vector<double> &distCoeffs);
    cv::Mat getCameraMatrix () const;
    std::vector<double> getDistCoeffs () const;

    void setDisplayMode (bool mode);

protected:
    QDoubleSpinBox *spinBoxFx;
    QDoubleSpinBox *spinBoxFy;
    QDoubleSpinBox *spinBoxCx;
    QDoubleSpinBox *spinBoxCy;
    QDoubleSpinBox *spinBoxK1;
    QDoubleSpinBox *spinBoxK2;
    QDoubleSpinBox *spinBoxP1;
    QDoubleSpinBox *spinBoxP2;
    QDoubleSpinBox *spinBoxK3;
    QDoubleSpinBox *spinBoxK4;
    QDoubleSpinBox *spinBoxK5;
    QDoubleSpinBox *spinBoxK6;
};


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL


#endif
