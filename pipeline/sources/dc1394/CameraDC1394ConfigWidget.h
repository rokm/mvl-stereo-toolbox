/*
 * DC1394 Camera: config widget
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

#ifndef CAMERA_DC1394_CONFIG_WIDGET_H
#define CAMERA_DC1394_CONFIG_WIDGET_H

#include <QtGui>


class CameraDC1394;

class CameraDC1394ConfigWidget : public QWidget
{
    Q_OBJECT
    
public:
    CameraDC1394ConfigWidget (CameraDC1394 *, QWidget * = 0);
    virtual ~CameraDC1394ConfigWidget ();

protected slots:
    void captureButtonToggled (bool);
    void updateCameraState ();

    void modeChanged (int);
    void framerateChanged (int);

    void updateParameters ();

protected:
    void addFeatureWidgets ();

protected:
    CameraDC1394 *camera;

    QPushButton *pushButtonCapture;

    QComboBox *comboBoxMode;
    QComboBox *comboBoxFramerate;
};

#endif
