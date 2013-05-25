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

#include "GuiStereoMethod.h"

#include "ImageDisplayWidget.h"

#include "StereoPipeline.h"
#include "StereoMethod.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/gpu/gpu.hpp>


GuiStereoMethod::GuiStereoMethod (StereoPipeline *p, QList<StereoMethod *> &m, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), methods(m)
{
    setWindowTitle("Stereo method");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *pushButton;
    QComboBox *comboBox;
    QLabel *label;
    QDoubleSpinBox *spinBoxD;
    QHBoxLayout *box;    
    
    layout->addLayout(buttonsLayout, 0, 0, 1, 2);

    buttonsLayout->addStretch();

    // Input image scaling
    box = new QHBoxLayout();
    box->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->addLayout(box);

    label = new QLabel("Input scaling", this);
    label->setToolTip("Input image scaling to speed up method or accomodate memory constraints.");
    box->addWidget(label);
    
    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setSingleStep(0.10);
    spinBoxD->setRange(0.01, 1.0);
    spinBoxD->setValue(pipeline->getStereoInputScaling());
    connect(spinBoxD, SIGNAL(valueChanged(double)), pipeline, SLOT(setStereoInputScaling(double)));
    connect(pipeline, SIGNAL(stereoInputScalingChanged(double)), spinBoxD, SLOT(setValue(double)));
    box->addWidget(spinBoxD);
    spinBoxInputScaling = spinBoxD;

    // Disparity image display type
    box = new QHBoxLayout();
    box->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->addLayout(box);

    label = new QLabel("Display type", this);
    label->setToolTip("Disparity image display type");
    box->addWidget(label);
    
    comboBox = new QComboBox(this);
    comboBox->addItem("Raw", DisplayRawDisparity);
    comboBox->setItemData(0, "Raw grayscale disparity.", Qt::ToolTipRole);
    comboBox->addItem("Dynamic range", DisplayDynamicDisparity);
    comboBox->setItemData(1, "Grayscale disparity scaled to min/max value.", Qt::ToolTipRole);
    if (cv::gpu::getCudaEnabledDeviceCount()) {
        // This one requires CUDA...
        comboBox->addItem("Color (GPU)", DisplayColorDisparity);
        comboBox->setItemData(2, "HSV disparity (computed on GPU).", Qt::ToolTipRole);
    }    
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateImage()));
    box->addWidget(comboBox);
    comboBoxDisplayType = comboBox;

    // Save
    pushButton = new QPushButton("Save disp.", this);
    pushButton->setToolTip("Save disparity image.");
    connect(pushButton, SIGNAL(released()), this, SLOT(saveImage()));
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveImage = pushButton;

    buttonsLayout->addStretch();

    // Disparity image
    displayDisparityImage = new ImageDisplayWidget("Disparity image", this);
    displayDisparityImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayDisparityImage, 2, 1);

    // Methods
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(scrollArea, 2, 0);
    
    QTabWidget *tabWidget = new QTabWidget(scrollArea);
    tabWidget->setTabPosition(QTabWidget::West);

    scrollArea->setWidget(tabWidget);
    scrollArea->setWidgetResizable(true);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar, 3, 0, 1, 2);


    // Create config tabs
    for (int i = 0; i < methods.size(); i++) {
        tabWidget->addTab(methods[i]->getConfigWidget(), methods[i]->getShortName());
    }

    // Method selection
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setMethod(int)));
    setMethod(tabWidget->currentIndex());

    // Pipeline
    connect(pipeline, SIGNAL(disparityImageChanged()), this, SLOT(updateImage()));
}

GuiStereoMethod::~GuiStereoMethod ()
{
}

void GuiStereoMethod::setMethod (int i)
{
    if (i < 0 || i >= methods.size()) {
        qWarning() << "Method" << i << "does not exist!";
        return;
    }
    
    pipeline->setStereoMethod(methods[i]);
}


void GuiStereoMethod::updateImage ()
{
    const cv::Mat &disparity = pipeline->getDisparityImage();
    int numDisparities = pipeline->getNumberOfDisparityLevels();

    // Show image
    int displayType = comboBoxDisplayType->itemData(comboBoxDisplayType->currentIndex()).toInt();
    switch (displayType) {
        case DisplayRawDisparity: {
            // Raw grayscale disparity
            displayDisparityImage->setImage(disparity);
            break;
        }
        case DisplayDynamicDisparity: {
            // Grayscale with dynamically rescaled values
            double minVal, maxVal, scale;
            cv::Mat scaledDisp;

            cv::minMaxLoc(disparity, &minVal, &maxVal, 0, 0);
            scale = 255.0 / (maxVal-minVal);
            disparity.convertTo(scaledDisp, CV_8UC1, scale, minVal/scale);
            
            displayDisparityImage->setImage(scaledDisp);
            break;
        }
        case DisplayColorDisparity: {
            // Hue-color-coded disparity
            cv::gpu::GpuMat gpu_disp(disparity);
            cv::gpu::GpuMat gpu_disp_color;
            cv::Mat disp_color;
        
            cv::gpu::drawColorDisp(gpu_disp, gpu_disp_color, numDisparities);
            gpu_disp_color.download(disp_color);
    
            displayDisparityImage->setImage(disp_color);
        
            break;
        }
    }
        

    // If image is valid, display computation time
    if (disparity.data) {
        statusBar->showMessage(QString("Disparity image (%1x%2) computed in %3 milliseconds; dropped %4 frames.").arg(disparity.cols).arg(disparity.rows).arg(pipeline->getDisparityImageComputationTime()).arg(pipeline->getStereoDroppedFrames()));
    } else {
        statusBar->clearMessage();
    }
}



// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void GuiStereoMethod::saveImage ()
{
    // Make snapshot of image - because it can take a while to get
    // the filename...
    cv::Mat tmpImg;

    pipeline->getDisparityImage().copyTo(tmpImg);

    // Get filename
    QString fileName = QFileDialog::getSaveFileName(this, "Save disparity image");
    if (!fileName.isNull()) {
        QFileInfo tmpFileName(fileName);

        // Extension
        QString ext = tmpFileName.completeSuffix();
        if (ext.isEmpty()) {
            fileName += ".png";
        }

        // Create filename
        try {
            cv::imwrite(fileName.toStdString(), tmpImg);
        } catch (cv::Exception e) {
            qWarning() << "Failed to save image:" << QString::fromStdString(e.what());
        }
    }
}
