/*
 * DC1394 Source: feature widget
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__DC1394__FEATURE_WIDGET_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__DC1394__FEATURE_WIDGET_H

#include <QtWidgets>

#include <dc1394/dc1394.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceDC1394 {


class Camera;

class FeatureWidget : public QWidget
{
    Q_OBJECT

public:
    FeatureWidget (Camera *camera, const dc1394feature_info_t &feature, QWidget *parent = Q_NULLPTR);
    virtual ~FeatureWidget ();

protected:
    void updateParameters ();

protected:
    Camera *camera;
    dc1394feature_info_t feature;

    QTimer *updateTimer;

    QSpinBox *spinBoxValue;
    QComboBox *comboBoxMode;
    QDoubleSpinBox *spinBoxAbsoluteValue;
};


} // SourceDC1394
} // Pipeline
} // StereoToolbox
} // MVL


#endif
