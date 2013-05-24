/*
 * MVL Stereo Toolbox: calibration GUI
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

#include "GuiCalibration.h"

#include "ImagePairDisplayWidget.h"

#include "StereoPipeline.h"
#include "StereoCalibration.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


GuiCalibration::GuiCalibration (StereoPipeline *p, StereoCalibration *c, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), calibration(c)
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
    pushButton->setToolTip("Calibrate from list of images.");
    connect(pushButton, SIGNAL(released()), this, SLOT(doCalibration()));
    buttonsLayout->addWidget(pushButton);
    pushButtonCalibrate = pushButton;

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

    // Calibration
    connect(calibration, SIGNAL(stateChanged(bool)), this, SLOT(updateState()));
    updateState();

    // Pattern dialog
    patternDialog = new CalibrationPatternDialog(this);
}

GuiCalibration::~GuiCalibration ()
{
}

void GuiCalibration::updateImage ()
{
    displayPair->setImagePair(pipeline->getLeftRectifiedImage(), pipeline->getRightRectifiedImage());
}

void GuiCalibration::updateState ()
{
    if (calibration->getState()) {
        statusBar->showMessage("Calibration set; rectifying input images.");

        displayPair->setImagePairROI(calibration->getLeftROI(), calibration->getRightROI());

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
void GuiCalibration::doCalibration ()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select calibration images or list file", QString(), "Images (*.jpg *.png *.bmp *.tif *.ppm *.pgm);; Text file (*.txt)");

    // If no files are chosen, stop
    if (!fileNames.size()) {
        return;
    }

    // If we are given a single file, assume it is a file list
    if (fileNames.size() == 1) {
        QFile listFile(fileNames[0]);
        if (listFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&listFile);
            QString line;
            
            // Clear file names
            fileNames.clear();

            while (1) {
                line = stream.readLine();
                if (line.isNull()) {
                    break;
                } else {
                    fileNames << line;
                }
            }

            if (!fileNames.size()) {
                throw QString("Image list is empty!");
            }
        } else {
            throw QString("Failed to open image list file \"%1\"!").arg(fileNames[0]);
        }
    }

    // Run pattern dialog
    if (patternDialog->exec() != QDialog::Accepted) {
        return;
    }

    CalibrationPattern pattern = patternDialog->getPattern();

    // Finally, do calibration
    calibration->calibrateFromImages(fileNames, pattern);
}

void GuiCalibration::importCalibration ()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load calibration from file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        calibration->loadStereoCalibration(fileName);
    }
}

void GuiCalibration::exportCalibration ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save calibration to file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        calibration->saveStereoCalibration(fileName);
    }
}

void GuiCalibration::clearCalibration ()
{
    calibration->clearStereoCalibration();
}


// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void GuiCalibration::saveImages ()
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
            qWarning() << "Failed to save images:" << QString::fromStdString(e.what());
        }
    }
}



//
//
//

CalibrationPatternDialog::CalibrationPatternDialog (QWidget *parent)
    : QDialog(parent)
{
    QFormLayout *layout = new QFormLayout(this);
    setLayout(layout);

    setWindowTitle("Calibration pattern");

    QLabel *label;
    QFrame *separator;
    QDialogButtonBox *buttonBox;

    // Pattern width
    label = new QLabel("Pattern width", this);
    label->setToolTip("Pattern width:\n"
                      " - chessboard: number of inside corners in horizontal direction\n"
                      " - circle grid: number of circles in horizontal direction\n"
                      " - asymmetric circle grid: number of circles in first column (*vertical* direction!)");
    
    spinBoxPatternWidth = new QSpinBox(this);
    spinBoxPatternWidth->setRange(1, 1000);
    spinBoxPatternWidth->setValue(19);
    layout->addRow(label, spinBoxPatternWidth);

    // Pattern height
    label = new QLabel("Pattern height", this);
    label->setToolTip("Pattern width:\n"
                      " - chessboard: number of inside corners in vertical direction\n"
                      " - circle grid: number of circles in vertical direction\n"
                      " - asymmetric circle grid: total number of circles in first *two* rows (*horizontal* direction!)");
    
    spinBoxPatternHeight = new QSpinBox(this);
    spinBoxPatternHeight->setRange(1, 1000);
    spinBoxPatternHeight->setValue(12);
    layout->addRow(label, spinBoxPatternHeight);

    // Element size
    label = new QLabel("Element size", this);
    label->setToolTip("Size of pattern elements:\n"
                      " - chessboard: square size (distance between two corners)"
                      " - circle grid: distance between circle centers"
                      " - asymmetric circle grid: distance between circle centers");

    spinBoxElementSize = new QDoubleSpinBox(this);
    spinBoxElementSize->setRange(1.0, 1000.0);
    spinBoxElementSize->setValue(25.0);
    spinBoxElementSize->setSuffix(" mm");
    layout->addRow(label, spinBoxElementSize);

    // Pattern type
    label = new QLabel("Pattern type", this);
    label->setToolTip("Calibration pattern type");

    comboBoxPatternType = new QComboBox(this);
    comboBoxPatternType->addItem("Chessboard", CalibrationPattern::Chessboard);
    comboBoxPatternType->addItem("Circle grid", CalibrationPattern::Circles);
    comboBoxPatternType->addItem("Asymmetric circle grid", CalibrationPattern::AsymmetricCircles);
    comboBoxPatternType->setCurrentIndex(0);
    layout->addRow(label, comboBoxPatternType);

    // Separator
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addRow(separator);

    // Max image scale level
    label = new QLabel("Image scale levels", this);
    label->setToolTip("Maximum image scale level. If pattern is not found at original image size,\n"
                      "image is upsampled and search is repeated. The image scale is 1.0 + level*scaleIncrement,\n"
                      "where level goes from 0 to imageScaleLevels. Set this variable to 0 to disable\n"
                      "multi-scale search.");

    spinBoxScaleLevels = new QSpinBox(this);
    spinBoxScaleLevels->setRange(0, 100);
    spinBoxScaleLevels->setValue(0);
    layout->addRow(label, spinBoxScaleLevels);

    // Max image scale level
    label = new QLabel("Scale increment", this);
    label->setToolTip("Scale increment for multi-scale pattern search. For details, see description of \n"
                      "image scale levels.");

    spinBoxScaleIncrement = new QDoubleSpinBox(this);
    spinBoxScaleIncrement->setRange(0.0, 2.0);
    spinBoxScaleIncrement->setSingleStep(0.05);
    spinBoxScaleIncrement->setValue(0.25);
    layout->addRow(label, spinBoxScaleIncrement);

    // Separator
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addRow(separator);

    // Show results
    checkBoxShowResult = new QCheckBox("Show results", this);
    checkBoxShowResult->setToolTip("Show detection results and allow user to discard them.");
    checkBoxShowResult->setChecked(true);
    layout->addRow(checkBoxShowResult);

    // Separator
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addRow(separator);

    // Button box
    buttonBox = new QDialogButtonBox(this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addRow(buttonBox);
    
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->addButton(QDialogButtonBox::Cancel);
}
    
CalibrationPatternDialog::~CalibrationPatternDialog ()
{
}


CalibrationPattern CalibrationPatternDialog::getPattern () const
{
    return CalibrationPattern(
        spinBoxPatternWidth->value(),
        spinBoxPatternHeight->value(),
        spinBoxElementSize->value(),
        (CalibrationPattern::PatternType)comboBoxPatternType->itemData(comboBoxPatternType->currentIndex()).toInt(),
        spinBoxScaleLevels->value(),
        spinBoxScaleIncrement->value()
    );
}

bool CalibrationPatternDialog::getShowResult () const
{
    return checkBoxShowResult->checkState() == Qt::Checked;
}
