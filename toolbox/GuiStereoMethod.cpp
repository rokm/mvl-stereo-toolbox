/*
 * MVL Stereo Toolbox: stereo method GUI
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

#include "GuiStereoMethod.h"

#include "ImageDisplayWidget.h"

#include "StereoPipeline.h"
#include "StereoMethod.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifdef HAVE_OPENCV_GPU
#include <opencv2/gpu/gpu.hpp>
#endif


GuiStereoMethod::GuiStereoMethod (StereoPipeline *p, QList<StereoMethod *> &m, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), methods(m)
{
    setWindowTitle("Stereo method");
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
    QDoubleSpinBox *spinBoxD;
    QHBoxLayout *box;    
    
    layout->addLayout(buttonsLayout);

    buttonsLayout->addStretch();

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
#ifdef HAVE_OPENCV_GPU
    if (cv::gpu::getCudaEnabledDeviceCount()) {
        // This one requires CUDA...
        comboBox->addItem("Color (GPU)", DisplayColorGpuDisparity);
        comboBox->setItemData(1, "HSV disparity (computed on GPU).", Qt::ToolTipRole);
    }
#endif
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateImage()));
    box->addWidget(comboBox);
    comboBoxDisplayType = comboBox;

    // Save
    pushButton = new QPushButton("Save disp.", this);
    pushButton->setToolTip("Save disparity image.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(saveImage()));
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveImage = pushButton;

    buttonsLayout->addStretch();

    // Splitter - disparity image and methods
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(splitter);
    
    // Methods
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->resize(400, 600); // Make sure scroll area has some size
    splitter->addWidget(scrollArea);
    
    QTabWidget *tabWidget = new QTabWidget(scrollArea);
    tabWidget->setTabPosition(QTabWidget::West);

    scrollArea->setWidget(tabWidget);
    scrollArea->setWidgetResizable(true);
    
    // Disparity image
    displayDisparityImage = new ImageDisplayWidget("Disparity image", this);
    displayDisparityImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    displayDisparityImage->resize(400, 600); // Make sure scroll area has some size
    splitter->addWidget(displayDisparityImage);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);


    // Create config tabs
    for (int i = 0; i < methods.size(); i++) {
        tabWidget->addTab(methods[i]->createConfigWidget(this), methods[i]->getShortName());
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
#ifdef HAVE_OPENCV_GPU
        case DisplayColorGpuDisparity: {
            // Hue-color-coded disparity
            cv::gpu::GpuMat gpu_disp(disparity);
            cv::gpu::GpuMat gpu_disp_color;
            cv::Mat disp_color;
        
            cv::gpu::drawColorDisp(gpu_disp, gpu_disp_color, numDisparities);
            gpu_disp_color.download(disp_color);
    
            displayDisparityImage->setImage(disp_color);
        
            break;
        }
#endif
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
