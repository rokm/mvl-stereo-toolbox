/*
 * MVL Stereo Toolbox: rectification window
 * Copyright (C) 2013-2015 Rok Mandeljc
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

#include <stereo-pipeline/pipeline.h>
#include <stereo-pipeline/rectification.h>
#include <stereo-pipeline/utils.h>
#include <stereo-widgets/image_pair_display_widget.h>

#include <opencv2/imgcodecs.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {


WindowRectification::WindowRectification (Pipeline::Pipeline *p, Pipeline::Rectification *r, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), rectification(r)
{
    setWindowTitle("Rectification");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    // Buttons
    QGridLayout *buttonsLayout = new QGridLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    QPushButton *pushButton;

    layout->addLayout(buttonsLayout, 0, 0, 1, 2);

    // Calibration buttons
    pushButton = new QPushButton("Calibrate");
    pushButton->setToolTip("Run calibration wizard.");
    connect(pushButton, &QPushButton::clicked, this, &WindowRectification::runCalibrationWizard);
    buttonsLayout->addWidget(pushButton, 0, 0, 1, 1);
    pushButtonWizard = pushButton;

    pushButton = new QPushButton("Clear calibration");
    pushButton->setToolTip("Clear current calibration.");
    connect(pushButton, &QPushButton::clicked, this, &WindowRectification::clearCalibration);
    buttonsLayout->addWidget(pushButton, 0, 1, 1, 1);
    pushButtonClear = pushButton;

    pushButton = new QPushButton("Import calibration");
    pushButton->setToolTip("Import calibration from file.");
    connect(pushButton, &QPushButton::clicked, this, &WindowRectification::importCalibration);
    buttonsLayout->addWidget(pushButton, 1, 0, 1, 1);
    pushButtonImport = pushButton;

    pushButton = new QPushButton("Export calibration");
    pushButton->setToolTip("Export current calibration to file.");
    connect(pushButton, &QPushButton::clicked, this, &WindowRectification::exportCalibration);
    buttonsLayout->addWidget(pushButton, 1, 1, 1, 1);
    pushButtonExport = pushButton;

    // Spacer
    buttonsLayout->addItem(new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 2, 2, 1);

    // Rectification settings
    pushButton = new QPushButton("Rectification settings");
    pushButton->setToolTip("Set rectification settings.");
    connect(pushButton, &QPushButton::clicked, this, &WindowRectification::modifyRectificationSettings);
    buttonsLayout->addWidget(pushButton, 0, 3, 1, 1);
    pushButtonRectificationSettings = pushButton;

    // ROI
    pushButton = new QPushButton("ROI");
    pushButton->setToolTip("Modify ROI on rectified images.");
    connect(pushButton, &QPushButton::clicked, this, &WindowRectification::modifyRoi);
    buttonsLayout->addWidget(pushButton, 1, 3, 1, 1);
    pushButtonRoi = pushButton;

    // Spacer
    buttonsLayout->addItem(new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 4, 2, 1);

    // Visualization type
    QHBoxLayout *box = new QHBoxLayout();
    box->setContentsMargins(0, 0, 0, 0);
    box->setSpacing(2);
    buttonsLayout->addLayout(box, 0, 5, 1, 1);

    QLabel *label = new QLabel("Vizualization type: ", this);
    label->setToolTip("Rectified pair vizualization type");
    box->addWidget(label);

    QComboBox *comboBox = new QComboBox(this); // Filled in by disparity image display widget!
    box->addWidget(comboBox);
    comboBoxVisualizationMethod = comboBox;

    comboBoxVisualizationMethod->addItem("Image pair", 0);
    comboBoxVisualizationMethod->setItemData(0, "Image pair", Qt::ToolTipRole);
    comboBoxVisualizationMethod->addItem("Anaglyph", 1);
    comboBoxVisualizationMethod->setItemData(1, "Anaglyph", Qt::ToolTipRole);

    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &WindowRectification::updateImage);

    // Save
    pushButton = new QPushButton("Save rectified pair / anaglyph");
    pushButton->setToolTip("Save rectified image pair or anaglyph, depending on visualization settings.");
    connect(pushButton, &QPushButton::clicked, this, &WindowRectification::saveImages);
    buttonsLayout->addWidget(pushButton, 1, 5, 1, 1);
    pushButtonSaveImages = pushButton;

    // Rectified image pair
    displayPair = new Widgets::ImagePairDisplayWidget("Rectified image pair", this);
    displayPair->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayPair, 1, 0);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar, 2, 0, 1, 2);

    checkBoxRectifyImages = new QCheckBox("Rectify images", statusBar);
    checkBoxRectifyImages->setToolTip("Allows image rectification to be turned off even when stereo calibration is loaded. Useful in\n"
                                "cases when input images are already rectified, and calibration is used for reprojection.");
    checkBoxRectifyImages->setChecked(rectification->getPerformRectification());
    connect(checkBoxRectifyImages, &QCheckBox::toggled, rectification, &Pipeline::Rectification::setPerformRectification);
    connect(rectification, &Pipeline::Rectification::performRectificationChanged, checkBoxRectifyImages, &QCheckBox::setChecked);
    statusBar->addPermanentWidget(checkBoxRectifyImages);

    // Pipeline
    connect(pipeline, &Pipeline::Pipeline::rectifiedImagesChanged, this, &WindowRectification::updateImage);

    // Roi dialog
    dialogRoi = new RoiDialog(this);

    // Rectification settings dialog
    dialogSettings = new RectificationSettingsDialog(this);

    // Rectification
    connect(rectification, &Pipeline::Rectification::stateChanged, this, &WindowRectification::updateState);
    updateState();

    // Calibration wizard
    wizard = new CalibrationWizard(this);

    // Pipeline's error signalization
    connect(pipeline, &Pipeline::Pipeline::error, this, [this] (int errorType, const QString &errorMessage) {
        if (errorType == Pipeline::Pipeline::ErrorRectification) {
            QMessageBox::warning(this, "Rectification Error", errorMessage);
        }
    });
}

WindowRectification::~WindowRectification ()
{
}

void WindowRectification::updateImage ()
{
    // Set image, based on selected visualization type
    int visualizationType = comboBoxVisualizationMethod->itemData(comboBoxVisualizationMethod->currentIndex()).toInt();

    if (visualizationType == VisualizationImagePair) {
        // Image pair
        displayPair->setImagePair(pipeline->getLeftRectifiedImage(), pipeline->getRightRectifiedImage());
    } else if (visualizationType == VisualizationAnaglyph) {
        // Anaglyph
        Utils::createAnaglyph(pipeline->getLeftRectifiedImage(), pipeline->getRightRectifiedImage(), anaglyphImage);
        displayPair->setImage(anaglyphImage);
    }

    // Update status bar
    if (rectification->getState()) {
        if (rectification->getPerformRectification()) {
            statusBar->showMessage(QString("Calibration set (estimated baseline: %1 mm); rectifying input images (%2 milliseconds).").arg(rectification->getStereoBaseline(), 0, 'f', 0).arg(pipeline->getRectificationTime()));
        } else {
            statusBar->showMessage(QString("Calibration set (estimated baseline: %1 mm); passing input images through.").arg(rectification->getStereoBaseline(), 0, 'f', 0));
        }
    } else {
        statusBar->showMessage("Calibration not set; passing input images through.");
    }
}

void WindowRectification::updateState ()
{
    if (rectification->getState()) {
        pushButtonClear->setEnabled(true);
        pushButtonExport->setEnabled(true);
    } else {
        pushButtonClear->setEnabled(false);
        pushButtonExport->setEnabled(false);
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
    QString fileName = QFileDialog::getOpenFileName(this, "Load calibration from file", lastCalibrationFilename, "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        lastCalibrationFilename = fileName; // Store filename
        try {
            rectification->loadStereoCalibration(fileName);
        } catch (QString &e) {
            QMessageBox::warning(this, "Error", "Failed to import calibration: " + e);
        }
    }
}

void WindowRectification::exportCalibration ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save calibration to file", lastCalibrationFilename, "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        lastCalibrationFilename = fileName; // Store filename

        QString ext = QFileInfo(fileName).completeSuffix();
        if (ext.isEmpty()) {
            ext = "yml";
            fileName += "." + ext;
        }

        try {
            rectification->saveStereoCalibration(fileName);
        } catch (QString &e) {
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

void WindowRectification::modifyRectificationSettings ()
{
    // Update dialog
    dialogSettings->setAlpha(rectification->getAlpha());
    dialogSettings->setZeroDisparity(rectification->getZeroDisparity());

    // Run dialog
    if (dialogSettings->exec() == QDialog::Accepted) {
        rectification->setAlpha(dialogSettings->getAlpha());
        rectification->setZeroDisparity(dialogSettings->getZeroDisparity());
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

    // Save image pair or anaglyph, based on selected visualization type
    int visualizationType = comboBoxVisualizationMethod->itemData(comboBoxVisualizationMethod->currentIndex()).toInt();

    QString dialogTitle;
    if (visualizationType == VisualizationImagePair) {
        dialogTitle = "Save rectified image pair";
    } else if (visualizationType == VisualizationAnaglyph) {
        dialogTitle = "Save anaglyph";
    }

    // Get filename
    QString fileName = QFileDialog::getSaveFileName(this, dialogTitle);
    if (fileName.isNull()) {
        return;
    }


    QFileInfo tmpFileName(fileName);

    // Extension
    QString ext = tmpFileName.completeSuffix();
    if (ext.isEmpty()) {
        ext = "jpg";
    }

    if (visualizationType == VisualizationImagePair) {
        // Create filename
        QString fileNameLeft = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "L" + "." + ext;
        QString fileNameRight = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "R" + "." + ext;

        try {
            cv::imwrite(fileNameLeft.toStdString(), tmpImg1);
            cv::imwrite(fileNameRight.toStdString(), tmpImg2);
        } catch (cv::Exception &e) {
            QMessageBox::warning(this, "Error", "Failed to save rectified image pair: " + QString::fromStdString(e.what()));
        }
    } else if (visualizationType == VisualizationAnaglyph) {
        QString fileNameAnaglyph = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "." + ext;

        cv::Mat tmpAnaglyph;
        Utils::createAnaglyph(tmpImg1, tmpImg2, tmpAnaglyph);

        try {
            cv::imwrite(fileNameAnaglyph.toStdString(), tmpAnaglyph);
        } catch (cv::Exception &e) {
            QMessageBox::warning(this, "Error", "Failed to save anaglyph: " + QString::fromStdString(e.what()));
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
    connect(checkBox, &QCheckBox::stateChanged, this, &RoiDialog::refreshDialog);
    checkBoxEnabled = checkBox;

    layout->addRow(checkBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Center
    checkBox = new QCheckBox("Center ROI", this);
    checkBox->setToolTip("Whether to center ROI and compute offsets automatically.");
    connect(checkBox, &QCheckBox::stateChanged, this, &RoiDialog::refreshDialog);
    checkBoxCenter = checkBox;

    layout->addRow(checkBox);

    // X
    label = new QLabel("X", this);
    label->setToolTip("ROI horizontal offset.");

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RoiDialog::refreshDialog);
    spinBoxX = spinBox;

    layout->addRow(label, spinBox);

    // Y
    label = new QLabel("Y", this);
    label->setToolTip("ROI vertical offset.");

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RoiDialog::refreshDialog);
    spinBoxY = spinBox;

    layout->addRow(label, spinBox);

    // Width
    label = new QLabel("Width", this);
    label->setToolTip("ROI width.");

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RoiDialog::refreshDialog);
    spinBoxW = spinBox;

    layout->addRow(label, spinBox);

    // Height
    label = new QLabel("Height", this);
    label->setToolTip("ROI height.");

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RoiDialog::refreshDialog);
    spinBoxH = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    layout->addRow(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RoiDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RoiDialog::reject);
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


// *********************************************************************
// *                   Rectification settings dialog                   *
// *********************************************************************
RectificationSettingsDialog::RectificationSettingsDialog (QWidget *parent)
    : QDialog(parent)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QCheckBox *checkBox;
    QDoubleSpinBox *spinBox;
    QFrame *line;

    setWindowTitle("Rectification settings");

    // Alpha
    label = new QLabel("Alpha", this);
    label->setToolTip("Free scaling parameter. Alpha=0 means that the rectified images are zoomed and shifted \n"
                      "so that only valid pixels are visible (no black areas after rectification). Alpha=1 means \n"
                      "that the rectified image is decimated and shifted so that all the pixels from the original \n"
                      "images from the cameras are retained in the rectified images (no source image pixels are lost).\n"
                      "If set to -1, default scaling is performed.");

    spinBox = new QDoubleSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-1, 1);
    spinBox->setDecimals(2);
    spinBox->setSingleStep(0.10);
    spinBoxAlpha = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Zero disparity
    checkBox = new QCheckBox("CALIB_ZERO_DISPARITY", this);
    checkBox->setToolTip("If the flag is set, the function makes the principal points of each camera have the same pixel coordinates in the rectified views.");
    //connect(checkBox, &QCheckBox::stateChanged, this, &RoiDialog::refreshDialog);
    checkBoxZeroDisparity = checkBox;

    layout->addRow(checkBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    layout->addRow(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RoiDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RoiDialog::reject);
}

RectificationSettingsDialog::~RectificationSettingsDialog ()
{
}


void RectificationSettingsDialog::setAlpha (float alpha)
{
    spinBoxAlpha->setValue(alpha);
}

float RectificationSettingsDialog::getAlpha () const
{
    return spinBoxAlpha->value();
}


void RectificationSettingsDialog::setZeroDisparity (bool value)
{
    checkBoxZeroDisparity->setChecked(value);
}

bool RectificationSettingsDialog::getZeroDisparity () const
{
    return checkBoxZeroDisparity->isChecked();
}


} // GUI
} // StereoToolbox
} // MVL
