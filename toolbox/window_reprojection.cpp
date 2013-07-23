/*
 * MVL Stereo Toolbox: reprojection window
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

#include "window_reprojection.h"
#include "image_display_widget.h"

#include <stereo_pipeline.h>
#include <stereo_reprojection.h>

#include <opencv2/highgui/highgui.hpp>


WindowReprojection::WindowReprojection (StereoPipeline *p, StereoReprojection *r, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), reprojection(r)
{
    setWindowTitle("Reprojection");
    resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);
    
    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    QPushButton *pushButton;
    QComboBox *comboBox;
    QLabel *label;
    QHBoxLayout *box;    
    
    layout->addLayout(buttonsLayout);

    buttonsLayout->addStretch();

    // Display image
    box = new QHBoxLayout();
    box->setContentsMargins(0, 0, 0, 0);
    box->setSpacing(2);
    buttonsLayout->addLayout(box);

    label = new QLabel("Image", this);
    label->setToolTip("Image to display in background");
    box->addWidget(label);
    
    comboBox = new QComboBox(this);
    comboBox->addItem("Disparity");
    comboBox->addItem("Left");
    comboBox->addItem("Right");
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateImage()));
    box->addWidget(comboBox);
    comboBoxImage = comboBox;

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

    connect(displayReprojectedImage, SIGNAL(coordinatesUnderMouseChanged(QVector3D)), this, SLOT(displayCoordinates(const QVector3D &)));

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);

    labelCoordinates = new QLabel(statusBar);
    statusBar->addPermanentWidget(labelCoordinates);

    // Pipeline
    connect(pipeline, SIGNAL(reprojectedImageChanged()), this, SLOT(updateImage()));
}

WindowReprojection::~WindowReprojection ()
{
}


void WindowReprojection::updateImage ()
{
    const cv::Mat &reprojectedImage = pipeline->getReprojectedImage();

    switch (comboBoxImage->currentIndex()) {
        case 0: {
            // Disparity
            displayReprojectedImage->setImage(pipeline->getDisparityImage(), reprojectedImage);
            break;
        }
        case 1: {
            // Left
            displayReprojectedImage->setImage(pipeline->getLeftRectifiedImage(), reprojectedImage);
            break;
        }
        case 2: {
            // Right
            displayReprojectedImage->setImage(pipeline->getRightRectifiedImage(), reprojectedImage);
            break;
        }
    }

    // If reprojected points are valid, display computation time
    if (reprojectedImage.data) {
        statusBar->showMessage(QString("Disparity image (%1x%2) reprojected in %3 milliseconds.").arg(reprojectedImage.cols).arg(reprojectedImage.rows).arg(pipeline->getReprojectionComputationTime()));
    } else {
        statusBar->clearMessage();
    }
}


void WindowReprojection::displayCoordinates (const QVector3D &coordinates)
{
    if (coordinates.isNull()) {
        labelCoordinates->setText("");
    } else {
        labelCoordinates->setText(QString("XYZ: %1, %2, %3").arg(coordinates.x()/1000, 0, 'f', 2).arg(coordinates.y()/1000, 0, 'f', 2).arg(coordinates.z()/1000, 0, 'f', 2));
    }
}
