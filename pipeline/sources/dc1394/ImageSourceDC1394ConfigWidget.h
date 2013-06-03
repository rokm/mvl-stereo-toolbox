/*
 * DC1394 Image Source: config widget
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

#ifndef IMAGE_SOURCE_DC1394_CONFIG_WIDGET_H
#define IMAGE_SOURCE_DC1394_CONFIG_WIDGET_H

#include <QtGui>


class ImageSourceDC1394;
class CameraDC1394;

class ImageSourceDC1394ConfigWidget : public QWidget
{
    Q_OBJECT
    
public:
    ImageSourceDC1394ConfigWidget (ImageSourceDC1394 *, QWidget * = 0);
    virtual ~ImageSourceDC1394ConfigWidget ();

protected slots:
    void deviceSelected (int);
    
protected:
    QWidget *createDeviceFrame (bool);

    void deviceSelected (QWidget *&, QFrame *&, QComboBox *&, int);

protected:
    ImageSourceDC1394 *source;

    // GUI
    QComboBox *comboBoxLeftDevice;
    QComboBox *comboBoxRightDevice;

    QPushButton *pushButtonCaptureLeftDevice;
    QPushButton *pushButtonCaptureRightDevice;

    QHBoxLayout *boxDevices;
    
    QFrame *frameLeftDevice;
    QFrame *frameRightDevice;

    QWidget *configLeftDevice;
    QWidget *configRightDevice;
};

#endif
