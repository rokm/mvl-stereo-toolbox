/*
 * DC1394 Camera: generic feature config widget
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

#ifndef FEATURE_WIDGET_H
#define FEATURE_WIDGET_H

#include <QtWidgets>

#include <dc1394/dc1394.h>


namespace SourceDC1394 {
    
class Camera;

class FeatureWidget : public QWidget
{
    Q_OBJECT

public:
    FeatureWidget (Camera *, const dc1394feature_info_t &, QWidget * = 0);
    virtual ~FeatureWidget ();

protected slots:
    void setValue (int);
    void setAbsoluteValue (double);
    void modeChanged (int);

    void updateParameters ();

protected:
    Camera *camera;
    dc1394feature_info_t feature;

    QTimer *updateTimer;

    QSpinBox *spinBoxValue;
    QComboBox *comboBoxMode;
    QDoubleSpinBox *spinBoxAbsoluteValue;
};

}

#endif
