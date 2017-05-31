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
    : QWidget(parent, Qt::Window), pipeline(p), rectification(r),
      numDroppedFrames(0)
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

    // Rectification settings dialog
    dialogSettings = new RectificationSettingsDialog(this);

    // Rectification
    connect(rectification, &Pipeline::Rectification::calibrationChanged, this, &WindowRectification::updateButtonsState);
    updateButtonsState();

    // Calibration wizard
    wizard = new CalibrationWizard(this);

    // Pipeline's error signalization
    connect(pipeline, &Pipeline::Pipeline::error, this, [this] (int errorType, const QString &errorMessage) {
        if (errorType == Pipeline::Pipeline::ErrorRectification) {
            QMessageBox::warning(this, "Rectification Error", errorMessage);
        }
    });

    // Rectification dropped frames counter
    connect(pipeline, &Pipeline::Pipeline::rectificationFrameDropped, this, [this] (int count) {
        numDroppedFrames = count;
        updateStatusBar();
    });
}

WindowRectification::~WindowRectification ()
{
}

void WindowRectification::updateStatusBar ()
{
    // Update status bar
    if (rectification->isCalibrationValid()) {
        if (rectification->getPerformRectification()) {
            statusBar->showMessage(QString("Calibration set (est. baseline: %1 mm) - rectifying images. FPS: %2, dropped %3 frames, operation time: %4 ms.")
                .arg(rectification->getStereoBaseline(), 0, 'f', 0)
                .arg(pipeline->getRectificationFramerate(), 0, 'f', 2)
                .arg(numDroppedFrames)
                .arg(pipeline->getRectificationTime())
            );
        } else {
            statusBar->showMessage(QString("Calibration set (est. baseline: %1 mm) - passing through. FPS: %2, dropped %3 frames, operation time: %4 ms.")
                .arg(rectification->getStereoBaseline(), 0, 'f', 0)
                .arg(pipeline->getRectificationFramerate(), 0, 'f', 2)
                .arg(numDroppedFrames)
                .arg(pipeline->getRectificationTime())
            );
        }
    } else {
        statusBar->showMessage(QString("Calibration not set - passing through. FPS: %1, dropped %2 frames, operation time: %3 ms.")
            .arg(pipeline->getRectificationFramerate(), 0, 'f', 2)
            .arg(numDroppedFrames)
            .arg(pipeline->getRectificationTime())
        );
    }
}

void WindowRectification::updateImage ()
{
    cv::Mat imageL, imageR;
    pipeline->getRectifiedImages(imageL, imageR);

    // Set image, based on selected visualization type
    int visualizationType = comboBoxVisualizationMethod->itemData(comboBoxVisualizationMethod->currentIndex()).toInt();

    if (visualizationType == VisualizationImagePair) {
        // Image pair
        displayPair->setImagePair(imageL, imageR);
    } else if (visualizationType == VisualizationAnaglyph) {
        // Anaglyph
        Utils::createAnaglyph(imageL, imageR, anaglyphImage);
        displayPair->setImage(anaglyphImage);
    }

    // Update status bar
    numDroppedFrames = 0; // Reset dropped frames counter
    updateStatusBar();
}

void WindowRectification::updateButtonsState ()
{
    if (rectification->isCalibrationValid()) {
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
    cv::Mat imageLeft, imageRight;
    pipeline->getRectifiedImages(imageLeft, imageRight);

    // Make sure images are actually available
    if (imageLeft.empty() || imageRight.empty()) {
        QMessageBox::information(this, "No data", "No data to export!");
        return;
    }

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
        ext = "png";
    }

    if (visualizationType == VisualizationImagePair) {
        // Create filename
        QString fileNameLeft = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "L" + "." + ext;
        QString fileNameRight = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "R" + "." + ext;

        try {
            cv::imwrite(fileNameLeft.toStdString(), imageLeft);
            cv::imwrite(fileNameRight.toStdString(), imageRight);
        } catch (const cv::Exception &e) {
            QMessageBox::warning(this, "Error", "Failed to save rectified image pair: " + QString::fromStdString(e.what()));
        }
    } else if (visualizationType == VisualizationAnaglyph) {
        QString fileNameAnaglyph = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "." + ext;

        cv::Mat anaglyph;
        Utils::createAnaglyph(imageLeft, imageRight, anaglyph);

        try {
            cv::imwrite(fileNameAnaglyph.toStdString(), anaglyph);
        } catch (const cv::Exception &e) {
            QMessageBox::warning(this, "Error", "Failed to save anaglyph: " + QString::fromStdString(e.what()));
        }
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
    checkBoxZeroDisparity = checkBox;

    layout->addRow(checkBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    layout->addRow(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RectificationSettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &RectificationSettingsDialog::reject);
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
