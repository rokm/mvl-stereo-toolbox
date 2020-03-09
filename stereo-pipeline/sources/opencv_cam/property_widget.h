/*
 * OpenCV Camera Source: property widget
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__OPENCV_CAM__PROPERTY_WIDGET_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__OPENCV_CAM__PROPERTY_WIDGET_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceOpenCvCam {


class Camera;

class PropertyWidget : public QDoubleSpinBox
{
    Q_OBJECT

public:
    PropertyWidget (Camera *camera, int property, bool integer_value, QWidget *parent = nullptr);
    virtual ~PropertyWidget ();

protected:
    void updateProperty ();

protected:
    Camera *camera;
    int property;

    QTimer *updateTimer;
};


} // SourceOpenCvCam
} // Pipeline
} // StereoToolbox
} // MVL


#endif
