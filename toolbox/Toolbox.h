/*
 * MVL Stereo Toolbox: main toolbox object and window
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

#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QtCore>
#include <QtGui>


class ImageSource;
class StereoRectification;
class StereoMethod;
class StereoPipeline;

class GuiImageSource;
class GuiRectification;
class GuiStereoMethod;

class Toolbox : public QWidget
{
    Q_OBJECT

public:
    Toolbox ();
    virtual ~Toolbox ();

protected slots:
    void showWindowImageSource ();
    void showWindowRectification ();
    void showWindowStereoMethod ();

    void setPushButtonImageSourceActiveState (bool);
    void setPushButtonRectificationActiveState (bool);
    void setPushButtonStereoMethodActiveState (bool);

protected:
    void createGui ();

    void loadPlugins ();
    
protected:
    GuiImageSource *windowImageSource;
    GuiRectification *windowRectification;
    GuiStereoMethod *windowStereoMethod;

    QPushButton *pushButtonImageSource;
    QPushButton *pushButtonImageSourceActive;
    QPushButton *pushButtonRectification;
    QPushButton *pushButtonRectificationActive;
    QPushButton *pushButtonStereoMethod;
    QPushButton *pushButtonStereoMethodActive;
    
    StereoPipeline *pipeline;
    StereoRectification *rectification;

    QList<ImageSource *> imageSources;
    QList<StereoMethod *> stereoMethods;
};


#endif
