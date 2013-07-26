/*
 * MVL Stereo Toolbox: rectification window
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

#include "window_rectification.h"
#include "calibration_wizard.h"
#include "image_display_widget.h"

#include <stereo_pipeline.h>
#include <stereo_rectification.h>

#include <opencv2/highgui/highgui.hpp>


WindowRectification::WindowRectification (StereoPipeline *p, StereoRectification *r, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), rectification(r)
{
    setWindowTitle("Rectification");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    QPushButton *pushButton;
    
    layout->addLayout(buttonsLayout, 0, 0, 1, 2);

    buttonsLayout->addStretch();

    pushButton = new QPushButton("Calibrate");
    pushButton->setToolTip("Run calibration wizard.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(runCalibrationWizard()));
    buttonsLayout->addWidget(pushButton);
    pushButtonWizard = pushButton;

    pushButton = new QPushButton("Import calib.");
    pushButton->setToolTip("Import calibration from file.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(importCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonImport = pushButton;

    pushButton = new QPushButton("Export calib.");
    pushButton->setToolTip("Export current calibration to file.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(exportCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonExport = pushButton;

    pushButton = new QPushButton("Clear calib.");
    pushButton->setToolTip("Clear current calibration.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(clearCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonClear = pushButton;

    pushButton = new QPushButton("ROI");
    pushButton->setToolTip("Modify ROI on rectified images.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(modifyRoi()));
    buttonsLayout->addWidget(pushButton);
    pushButtonRoi = pushButton;

    pushButton = new QPushButton("Save rectified images");
    pushButton->setToolTip("Save rectified image pair.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(saveImages()));
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

    // Roi dialog
    dialogRoi = new RoiDialog(this);

    // Rectification
    connect(rectification, SIGNAL(stateChanged(bool)), this, SLOT(updateState()));
    updateState();

    // Calibration wizard
    wizard = new CalibrationWizard(this);
}

WindowRectification::~WindowRectification ()
{
}

void WindowRectification::updateImage ()
{
    displayPair->setImagePair(pipeline->getLeftRectifiedImage(), pipeline->getRightRectifiedImage());
}

void WindowRectification::updateState ()
{
    if (rectification->getState()) {
        statusBar->showMessage(QString("Calibration set (estimated baseline: %1 mm); rectifying input images (%2 milliseconds).").arg(rectification->getStereoBaseline(), 0, 'f', 0).arg(pipeline->getRectificationTime()));
        pushButtonClear->setEnabled(true);
        pushButtonExport->setEnabled(true);
        pushButtonRoi->setEnabled(true);
    } else {
        statusBar->showMessage("Calibration not set; passing input images through.");
        pushButtonClear->setEnabled(false);
        pushButtonExport->setEnabled(false);
        pushButtonRoi->setEnabled(false);
    }
}


// *********************************************************************
// *                            Calibration                            *
// *********************************************************************
void WindowRectification::runCalibrationWizard ()
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

void WindowRectification::importCalibration ()
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

void WindowRectification::exportCalibration ()
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

void WindowRectification::clearCalibration ()
{
    rectification->clearStereoCalibration();
}

void WindowRectification::modifyRoi ()
{
    // Update image size and ROI
    dialogRoi->setImageSizeAndRoi(rectification->getImageSize(), rectification->getRoi());

    // Run dialog
    if (dialogRoi->exec() == QDialog::Accepted) {
        rectification->setRoi(dialogRoi->getRoi());
    }
}


// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void WindowRectification::saveImages ()
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


// *********************************************************************
// *                             ROI dialog                            *
// *********************************************************************
RoiDialog::RoiDialog (QWidget *parent)
    : QDialog(parent)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QCheckBox *checkBox;
    QSpinBox *spinBox;
    QFrame *line;

    setWindowTitle("ROI");

    // Enabled
    checkBox = new QCheckBox("ROI enabled", this);
    checkBox->setToolTip("ROI enabled or not.");
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(refreshDialog()));
    checkBoxEnabled = checkBox;

    layout->addRow(checkBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Center
    checkBox = new QCheckBox("Center ROI", this);
    checkBox->setToolTip("Whether to center ROI and compute offsets automatically.");
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(refreshDialog()));
    checkBoxCenter = checkBox;

    layout->addRow(checkBox);

    // X
    label = new QLabel("X", this);
    label->setToolTip("ROI horizontal offset.");

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(refreshDialog()));
    spinBoxX = spinBox;

    layout->addRow(label, spinBox);

    // Y
    label = new QLabel("Y", this);
    label->setToolTip("ROI vertical offset.");

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(refreshDialog()));
    spinBoxY = spinBox;

    layout->addRow(label, spinBox);
    
    // Width
    label = new QLabel("Width", this);
    label->setToolTip("ROI width.");

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(refreshDialog()));
    spinBoxW = spinBox;

    layout->addRow(label, spinBox);

    // Height
    label = new QLabel("Height", this);
    label->setToolTip("ROI height.");

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(refreshDialog()));
    spinBoxH = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    layout->addRow(buttonBox);    
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

RoiDialog::~RoiDialog ()
{
}


void RoiDialog::setImageSizeAndRoi (const cv::Size &size, const cv::Rect &currentRoi)
{
    // Store image size
    imageSize = size;

    // Set ROI values
    spinBoxX->setValue(currentRoi.x);
    spinBoxY->setValue(currentRoi.y);
    spinBoxW->setValue(currentRoi.width);
    spinBoxH->setValue(currentRoi.height);
    
    if (currentRoi == cv::Rect()) {
        checkBoxEnabled->setChecked(false);
    } else {
        checkBoxEnabled->setChecked(true);

        if (currentRoi.x == (imageSize.width - currentRoi.width)/2 && currentRoi.y == (imageSize.height - currentRoi.height)/2) {
            checkBoxCenter->setChecked(true);
        } else {
            checkBoxCenter->setChecked(false);
        }       
    }

    // Initial refresh
    refreshDialog();
}

void RoiDialog::refreshDialog ()
{
    // Enable/disable widgets
    if (checkBoxEnabled->isChecked()) {
        checkBoxCenter->setEnabled(true);
        if (checkBoxCenter->isChecked()) {
            spinBoxX->setEnabled(false);
            spinBoxY->setEnabled(false);
        } else {
            spinBoxX->setEnabled(true);
            spinBoxY->setEnabled(true);
        }
        spinBoxW->setEnabled(true);
        spinBoxH->setEnabled(true);
        
    } else {
        checkBoxCenter->setEnabled(false);
        spinBoxX->setEnabled(false);
        spinBoxY->setEnabled(false);
        spinBoxW->setEnabled(false);
        spinBoxH->setEnabled(false);
    }
    
    // Make sure that ROI offset + ROI dimension <= image dimension
    spinBoxX->setRange(0, imageSize.width - spinBoxW->value());
    spinBoxY->setRange(0, imageSize.height - spinBoxH->value());

    spinBoxW->setRange(0, imageSize.width - spinBoxX->value());
    spinBoxH->setRange(0, imageSize.height - spinBoxY->value());

    // Update values
    if (checkBoxEnabled->isChecked()) {
        if (checkBoxCenter->isChecked()) {
            spinBoxX->setValue((imageSize.width - spinBoxW->value())/2);
            spinBoxY->setValue((imageSize.height - spinBoxH->value())/2);
        }
    } else {
        spinBoxX->setValue(0);
        spinBoxY->setValue(0);
        spinBoxW->setValue(imageSize.width);
        spinBoxH->setValue(imageSize.height);
    }
}


cv::Rect RoiDialog::getRoi () const
{
    if (checkBoxEnabled->isChecked()) {
        return cv::Rect(spinBoxX->value(), spinBoxY->value(), spinBoxW->value(), spinBoxH->value());
    } else {
        return cv::Rect();
    }
}
