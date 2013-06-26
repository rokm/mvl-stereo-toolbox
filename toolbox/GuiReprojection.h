/*
 * MVL Stereo Toolbox: reprojection GUI
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

#ifndef GUI_REPROJECTION_H
#define GUI_REPROJECTION_H

#include <QtCore>
#include <QtGui>

#include "StereoReprojection.h"


class StereoReprojection;
class StereoPipeline;
class ReprojectedImageDisplayWidget;

class GuiReprojection : public QWidget
{
    Q_OBJECT

public:
    GuiReprojection (StereoPipeline *, StereoReprojection *, QWidget * = 0);
    virtual ~GuiReprojection ();

protected slots:
    void updateImage ();
    
protected:
    // Pipeline
    StereoPipeline *pipeline;
    StereoReprojection *reprojection;

    // GUI
    QPushButton *pushButtonUseGpu;

    ReprojectedImageDisplayWidget *displayReprojectedImage;
    
    QStatusBar *statusBar;
};

#endif
