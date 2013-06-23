/*
 * MVL Stereo Toolbox: reprojection/point cloud GUI
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

#include "GuiPointCloud.h"

#include "StereoPipeline.h"
#include "StereoReprojection.h"

#include <opencv2/highgui/highgui.hpp>


GuiPointCloud::GuiPointCloud (StereoPipeline *p, StereoReprojection *r, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), reprojection(r)
{
    setWindowTitle("Reprojection/point cloud");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);


    // PCL viewer
    
    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar, 2, 0, 1, 2);

    // Pipeline
    //connect(pipeline, SIGNAL(rectifiedImagesChanged()), this, SLOT(updateImage()));
    //connect(pipeline, SIGNAL(centerRoiChanged()), this, SLOT(updateRoi()));
}

GuiPointCloud::~GuiPointCloud ()
{
}
