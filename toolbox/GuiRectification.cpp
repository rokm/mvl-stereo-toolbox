/*
 * MVL Stereo Toolbox: rectification GUI
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

#include "GuiRectification.h"

#include "CalibrationWizard.h"
#include "ImagePairDisplayWidget.h"
#include "StereoPipeline.h"
#include "StereoRectification.h"

#include <opencv2/highgui/highgui.hpp>


GuiRectification::GuiRectification (StereoPipeline *p, StereoRectification *r, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), rectification(r)
{
    setWindowTitle("Stereo calibration");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *pushButton;
    
    layout->addLayout(buttonsLayout, 0, 0, 1, 2);

    buttonsLayout->addStretch();

    pushButton = new QPushButton("Calibrate");
    pushButton->setToolTip("Run calibration wizard.");
    connect(pushButton, SIGNAL(released()), this, SLOT(runCalibrationWizard()));
    buttonsLayout->addWidget(pushButton);
    pushButtonWizard = pushButton;

    pushButton = new QPushButton("Import calib.");
    pushButton->setToolTip("Import calibration from file.");
    connect(pushButton, SIGNAL(released()), this, SLOT(importCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonImport = pushButton;

    pushButton = new QPushButton("Export calib.");
    pushButton->setToolTip("Export current calibration to file.");
    connect(pushButton, SIGNAL(released()), this, SLOT(exportCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonExport = pushButton;

    pushButton = new QPushButton("Clear calib.");
    pushButton->setToolTip("Clear current calibration.");
    connect(pushButton, SIGNAL(released()), this, SLOT(clearCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonClear = pushButton;

    pushButton = new QPushButton("Save rectified images");
    pushButton->setToolTip("Save rectified image pair.");
    connect(pushButton, SIGNAL(released()), this, SLOT(saveImages()));
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveImages = pushButton;

    buttonsLayout->addStretch();

    // Rectified image pair
    displayPair = new ImagePairDisplayWidget("Rectified image pair", this);
    displayPair->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayPair, 1, 0);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar, 2, 0, 1, 2);

    // Pipeline
    connect(pipeline, SIGNAL(rectifiedImagesChanged()), this, SLOT(updateImage()));

    // Rectification
    connect(rectification, SIGNAL(stateChanged(bool)), this, SLOT(updateState()));
    updateState();

    // Calibration wizard
    wizard = new CalibrationWizard(this);
}

GuiRectification::~GuiRectification ()
{
}

void GuiRectification::updateImage ()
{
    displayPair->setImagePair(pipeline->getLeftRectifiedImage(), pipeline->getRightRectifiedImage());
}

void GuiRectification::updateState ()
{
    if (rectification->getState()) {
        statusBar->showMessage("Calibration set; rectifying input images.");

        displayPair->setImagePairROI(rectification->getLeftROI(), rectification->getRightROI());

        pushButtonClear->setEnabled(true);
        pushButtonExport->setEnabled(true);
    } else {
        statusBar->showMessage("Calibration not set; passing input images through.");

        displayPair->setImagePairROI(cv::Rect(), cv::Rect());

        pushButtonClear->setEnabled(false);
        pushButtonExport->setEnabled(false);
    }

    //updateImage();
}


// *********************************************************************
// *                            Calibration                            *
// *********************************************************************
void GuiRectification::runCalibrationWizard ()
{
    // Run calibration wizard
    wizard->restart();
    if (wizard->exec() == QDialog::Accepted) {
        QString fieldPrefix = "Stereo";
        // Get parameters and set them to rectification object
        rectification->setStereoCalibration(
            wizard->field(fieldPrefix + "CameraMatrix1").value<cv::Mat>(),
            wizard->field(fieldPrefix + "DistCoeffs1").value<cv::Mat>(),
            wizard->field(fieldPrefix + "CameraMatrix2").value<cv::Mat>(),
            wizard->field(fieldPrefix + "DistCoeffs2").value<cv::Mat>(),
            wizard->field(fieldPrefix + "R").value<cv::Mat>(),
            wizard->field(fieldPrefix + "T").value<cv::Mat>(),
            wizard->field(fieldPrefix + "ImageSize").value<cv::Size>()
        );        
    }
}

void GuiRectification::importCalibration ()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load calibration from file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        try {
            rectification->loadStereoCalibration(fileName);
        } catch (QString e) {
            QMessageBox::warning(this, "Error", "Failed to import calibration: " + e);
        }
    }
}

void GuiRectification::exportCalibration ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save calibration to file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        try {
            rectification->saveStereoCalibration(fileName);
        } catch (QString e) {
            QMessageBox::warning(this, "Error", "Failed to export calibration: " + e);
        }
    }
}

void GuiRectification::clearCalibration ()
{
    rectification->clearStereoCalibration();
}


// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void GuiRectification::saveImages ()
{
    // Make snapshot of images - because it can take a while to get
    // the filename...
    cv::Mat tmpImg1, tmpImg2;

    pipeline->getLeftRectifiedImage().copyTo(tmpImg1);
    pipeline->getRightRectifiedImage().copyTo(tmpImg2);
    
    // Get filename
    QString fileName = QFileDialog::getSaveFileName(this, "Save rectified images");
    if (!fileName.isNull()) {
        QFileInfo tmpFileName(fileName);

        // Extension
        QString ext = tmpFileName.completeSuffix();
        if (ext.isEmpty()) {
            ext = "jpg";
        }

        // Create filename
        QString fileNameLeft = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "L" + "." + ext;
        QString fileNameRight = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "R" + "." + ext;

        try {
            cv::imwrite(fileNameLeft.toStdString(), tmpImg1);
            cv::imwrite(fileNameRight.toStdString(), tmpImg2);
        } catch (cv::Exception e) {
            QMessageBox::warning(this, "Error", "Failed to save images: " + QString::fromStdString(e.what()));
        }
    }
}
