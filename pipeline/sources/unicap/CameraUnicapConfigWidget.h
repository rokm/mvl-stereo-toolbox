/*
 * Unicap Camera: config widget
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

#ifndef CAMERA_UNICAP_CONFIG_WIDGET_H
#define CAMERA_UNICAP_CONFIG_WIDGET_H

#include <QtGui>
#include <unicap.h>

class CameraUnicap;
class SizeWidget;

class CameraUnicapConfigWidget : public QWidget
{
    Q_OBJECT
    
public:
    CameraUnicapConfigWidget (CameraUnicap *, QWidget * = 0);
    virtual ~CameraUnicapConfigWidget ();

protected slots:
    void captureButtonToggled (bool);
    void updateCameraState ();

    void comboBoxFormatActivated (int);
    void comboBoxSizeActivated (int);
    void sizeWidgetChanged (unicap_rect_t);
    
    void updateFormat ();
    void updateSize ();

protected:
    void addPropertyWidgets ();

protected:
    CameraUnicap *camera;

    QPushButton *pushButtonCapture;

    QComboBox *comboBoxFormat;

    unicap_format_t format;

    QLabel *labelSize1;
    QComboBox *comboBoxSize;

    QLabel *labelSize2;
    SizeWidget *widgetSize;
};


class SizeWidget : public QWidget
{
    Q_OBJECT

public:
    SizeWidget (QWidget * = 0);
    virtual ~SizeWidget ();

    unicap_rect_t getSize () const;
    
public slots:
    void setSize (const unicap_rect_t &);

    void setValidSizeRange (const unicap_rect_t &, const unicap_rect_t &, int, int);

signals:
    void sizeChanged (unicap_rect_t);

protected:
    QSpinBox *spinBoxOffsetX;
    QSpinBox *spinBoxOffsetY;
    QSpinBox *spinBoxWidth;
    QSpinBox *spinBoxHeight;
};


#endif
