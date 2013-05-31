/*
 * MVL Stereo Toolbox: stereo method GUI
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

#ifndef GUI_STEREO_METHOD_H
#define GUI_STEREO_METHOD_H

#include <QtCore>
#include <QtGui>


class StereoPipeline;
class StereoMethod;

class ImageDisplayWidget;

class GuiStereoMethod : public QWidget
{
    Q_OBJECT

public:
    GuiStereoMethod (StereoPipeline *, QList<StereoMethod *> &, QWidget * = 0);
    virtual ~GuiStereoMethod ();

protected slots:
    void setMethod (int);
    
    void updateImage ();

    void saveImage ();

protected:
    enum {
        DisplayRawDisparity,
        DisplayDynamicDisparity,
        DisplayColorDisparity,
    };

    // Pipeline
    StereoPipeline *pipeline;
    QList<StereoMethod *> methods;

    // GUI
    QComboBox *comboBoxDisplayType;
    QPushButton *pushButtonSaveImage;
    
    ImageDisplayWidget *displayDisparityImage;

    QStatusBar *statusBar;
};

#endif
