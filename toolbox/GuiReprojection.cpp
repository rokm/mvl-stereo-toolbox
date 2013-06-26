/*
 * MVL Stereo Toolbox: reprojection GUI
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

#include "GuiReprojection.h"

#include "ImageDisplayWidget.h"

#include "StereoPipeline.h"
#include "StereoReprojection.h"

#include <opencv2/highgui/highgui.hpp>


GuiReprojection::GuiReprojection (StereoPipeline *p, StereoReprojection *r, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), reprojection(r)
{
    setWindowTitle("Reprojection");
    resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *pushButton;
    
    layout->addLayout(buttonsLayout);

    buttonsLayout->addStretch();

    // Use GPU
    pushButton = new QPushButton("Use GPU", this);
    pushButton->setToolTip("Use GPU for reprojection.");
    pushButton->setCheckable(true);
    pushButton->setChecked(reprojection->getUseGpu());
    connect(pushButton, SIGNAL(toggled(bool)), reprojection, SLOT(setUseGpu(bool)));
    connect(reprojection, SIGNAL(useGpuChanged(bool)), pushButton, SLOT(setChecked(bool)));
    buttonsLayout->addWidget(pushButton);
    pushButtonUseGpu = pushButton;
    
    buttonsLayout->addStretch();

    // Reprojected image viewer
    displayReprojectedImage = new ReprojectedImageDisplayWidget("Reprojected image", this);
    displayReprojectedImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    displayReprojectedImage->resize(400, 600); // Make sure scroll area has some size
    layout->addWidget(displayReprojectedImage);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);

    // Pipeline
    connect(pipeline, SIGNAL(reprojectedImageChanged()), this, SLOT(updateImage()));
}

GuiReprojection::~GuiReprojection ()
{
}


void GuiReprojection::updateImage ()
{
    displayReprojectedImage->setImage(pipeline->getDisparityImage(), pipeline->getReprojectedImage());
}
