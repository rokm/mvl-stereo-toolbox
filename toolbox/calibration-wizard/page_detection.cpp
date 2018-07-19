/*
 * MVL Stereo Toolbox: calibration wizard: pattern detection page
 * Copyright (C) 2013-2017 Rok Mandeljc
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

#include "page_detection.h"
#include "wizard.h"

#include <stereo-pipeline/calibration_pattern.h>
#include <stereo-pipeline/pipeline.h>
#include <stereo-widgets/calibration_pattern_display_widget.h>

#include <opencv2/imgcodecs.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                   Pattern detection page: common                  *
// *********************************************************************
PageDetection::PageDetection (const QString &fieldPrefixString, Pipeline::Pipeline *pipeline, QWidget *parent)
    : QWizardPage(parent),
      fieldPrefix(fieldPrefixString),
      autoProcess(false),
      autoProcessTimer(new QTimer(this)),
      calibrationPattern(new Pipeline::CalibrationPattern()),
      liveCapture(false),
      doLiveUpdate(false),
      pipeline(pipeline)
{
    setSubTitle("Calibration pattern detection");

    QFrame *separator;

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Label: the actual text is set by children
    labelCaption = new QLabel();
    labelCaption->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    labelCaption->setWordWrap(true);

    layout->addWidget(labelCaption);

    // Separator
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(separator);

    // NOTE: display widget is added by children

    // *** Status and buttons ***
    QHBoxLayout *hbox = new QHBoxLayout();
    layout->addLayout(hbox);

    labelStatus = new QLabel(this);
    hbox->addWidget(labelStatus, 1);

    // Process (only for live-capture)
    pushButtonProcess = new QPushButton("Process", this);
    connect(pushButtonProcess, &QPushButton::clicked, this, &PageDetection::processImage);
    hbox->addWidget(pushButtonProcess);

    // Auto (only for non-live-capture)
    pushButtonAuto = new QPushButton("Auto", this);
    pushButtonAuto->setCheckable(true);
    connect(pushButtonAuto, &QPushButton::toggled, this, [this] (bool enable) {
        // Enable/disable auto processing
        autoProcess = enable;

        if (enable) {
            if (imageCounter == 0) {
                // Start processing
                startProcessing();
            } else {
                // Automatically process current frame
                doAutomaticProcessing();
            }
        }
    });
    hbox->addWidget(pushButtonAuto);

    // Discard
    pushButtonDiscard = new QPushButton("Discard", this);
    connect(pushButtonDiscard, &QPushButton::clicked, this, [this] () {
        // Disable auto processing
        pushButtonAuto->setChecked(false);

        // Manual discard
        discardPattern();
    });
    hbox->addWidget(pushButtonDiscard);

    // Accept
    pushButtonAccept = new QPushButton("Accept", this);
    connect(pushButtonAccept, &QPushButton::clicked, this, [this] () {
        // Disable auto processing
        pushButtonAuto->setChecked(false);

        // Manual accept
        acceptPattern();
    });
    hbox->addWidget(pushButtonAccept);

    // Start (only for non-live-capture)
    pushButtonStart = new QPushButton("Start", this);
    connect(pushButtonStart, &QPushButton::clicked, this, &PageDetection::startProcessing);
    hbox->addWidget(pushButtonStart);

    // Auto-process timer
    autoProcessTimer->setSingleShot(true);
    connect(autoProcessTimer, &QTimer::timeout, this, &PageDetection::doAutomaticProcessing);

    // Live-capture
    connect(pipeline, &Pipeline::Pipeline::inputImagesChanged, this, &PageDetection::updateLiveCapture);

    // Fields
    registerField(fieldPrefix + "PatternImagePoints", this, "patternImagePoints");
    registerField(fieldPrefix + "PatternWorldPoints", this, "patternWorldPoints");
    registerField(fieldPrefix + "ImageSize", this, "imageSize");
}

PageDetection::~PageDetection ()
{
    delete calibrationPattern;
}


const std::vector<std::vector<cv::Point2f> > &PageDetection::getPatternImagePoints () const
{
    return patternImagePoints;
}

const std::vector<std::vector<cv::Point3f> > &PageDetection::getPatternWorldPoints () const
{
    return patternWorldPoints;
}

void PageDetection::setImageSize (const cv::Size &size)
{
    imageSize = size;
}

const cv::Size &PageDetection::getImageSize () const
{
    return imageSize;
}


void PageDetection::initializePage ()
{
    liveCapture = field(fieldPrefix + "LiveCapture").toBool();

    // Get fresh images list and reset the counter
    images = field(fieldPrefix + "Images").toStringList();
    imageCounter = 0;

    patternFound = false;

    // NOTE: images are cleared by children

    // Clear points
    patternImagePoints.clear();
    patternWorldPoints.clear();

    // Clear status label
    if (liveCapture) {
        labelStatus->setText("Press \"Process\" to capture and process image.");
    } else {
        labelStatus->setText("Press \"Start\" to begin.");
    }

    // Hide Accept and Discard buttons at first
    pushButtonAccept->hide();
    pushButtonDiscard->hide();

    // Show Process button (live capture); or Start and
    // Auto buttons (non-live)
    if (liveCapture) {
        pushButtonStart->hide();
        pushButtonStart->setEnabled(false);

        pushButtonAuto->hide();
        pushButtonAuto->setEnabled(false);

        pushButtonProcess->show();
        pushButtonProcess->setEnabled(true);
    } else {
        pushButtonStart->show();
        pushButtonStart->setEnabled(true);

        pushButtonAuto->show();
        pushButtonAuto->setEnabled(true);

        pushButtonProcess->hide();
        pushButtonProcess->setEnabled(false);
    }

    // Enable live capture flag for live capture
    if (liveCapture) {
        enableLiveUpdate();
    }

    // Set parameters for pattern detector
    calibrationPattern->setParameters(
        field(fieldPrefix + "PatternWidth").toInt(),
        field(fieldPrefix + "PatternHeight").toInt(),
        field(fieldPrefix + "ElementSize").toDouble(),
        (Pipeline::CalibrationPattern::PatternType)field(fieldPrefix + "PatternType").toInt(),
        field(fieldPrefix + "ScaleLevels").toInt(),
        field(fieldPrefix + "ScaleIncrement").toDouble()
    );
}


bool PageDetection::isComplete () const
{
    // We need at least six valid images/pairs (= world coordinate vectors)
    return patternWorldPoints.size() >= 6;
}

bool PageDetection::validatePage ()
{
    // Stop the auto-detection
    pushButtonAuto->setChecked(false);

    return QWizardPage::validatePage();
}


void PageDetection::startProcessing ()
{
    pushButtonStart->hide();
    pushButtonStart->setEnabled(false);

    pushButtonAccept->show();
    pushButtonAccept->setEnabled(true);

    pushButtonDiscard->show();
    pushButtonDiscard->setEnabled(true);

    // Process next
    processImage();
}

void PageDetection::doAutomaticProcessing ()
{
    // Make sure auto processing is still enabled
    if (!autoProcess) {
        return;
    }

    // Auto accept / discard
    if (patternFound) {
        acceptPattern();
    } else {
        discardPattern();
    }
}

void PageDetection::disableLiveUpdate ()
{
    // Disable live update
    doLiveUpdate = false;

    // Hide Process button
    pushButtonProcess->hide();

    pushButtonAccept->show();
    pushButtonDiscard->show();
}

void PageDetection::enableLiveUpdate ()
{
    // Enable live update
    doLiveUpdate = true;

    // Show Process button...
    pushButtonProcess->show();

    // ... and hide Accept/Discard buttons
    pushButtonAccept->hide();
    pushButtonDiscard->hide();

    // Manually update live capture
    updateLiveCapture();
}


// *********************************************************************
// *               Pattern detection page: single camera               *
// *********************************************************************
PageSingleCameraDetection::PageSingleCameraDetection (Pipeline::Pipeline *pipeline, QWidget *parent)
    : PageSingleCameraDetection("SingleCamera", pipeline, parent)
{
}

PageSingleCameraDetection::PageSingleCameraDetection (const QString &fieldPrefixString, Pipeline::Pipeline *pipeline, QWidget *parent)
    : PageDetection(fieldPrefixString, pipeline, parent)
{
    setTitle("Single camera calibration");

    labelCaption->setText(
        "The procedure will try to locate calibration pattern in each "
        "calibration image, displaying result at each step. Please accept "
        "or discard the result (for example, if the pattern is mislocated). "
        "Please note that to complete this step, the pattern must be located "
        "in at least six images."
    );


    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());

    // Display widget
    widgetImage = new Widgets::CalibrationPatternDisplayWidget("Invalid image!", this);
    widgetImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->insertWidget(2, widgetImage);
}

PageSingleCameraDetection::~PageSingleCameraDetection ()
{
}


int PageSingleCameraDetection::nextId () const
{
    return Wizard::PageId::SingleCameraCalibrationId;
}


void PageSingleCameraDetection::initializePage ()
{
    PageDetection::initializePage();

    // Clear image
    widgetImage->setImage(cv::Mat());
}


void PageSingleCameraDetection::acceptPattern ()
{
    // Append image coordinates
    patternImagePoints.push_back(currentImagePoints);

    // Append world coordinates
    patternWorldPoints.push_back(calibrationPattern->computePlanarCoordinates());

    imageCounter++;

    if (!liveCapture) {
        // Process next
        processImage();
    } else {
        // Re-enable live update
        enableLiveUpdate();
    }

    // We might be ready to go on...
    emit completeChanged();
}

void PageSingleCameraDetection::discardPattern ()
{
    // Skip the image
    imageCounter++;

    if (!liveCapture) {
        // Process next pair
        processImage();
    } else {
        // Re-enable live update
        enableLiveUpdate();
    }
}

void PageSingleCameraDetection::updateLiveCapture ()
{
    if (!doLiveUpdate) {
        return;
    }

    // Display new image
    cv::Mat image = getImageFromPipeline();
    widgetImage->setImage(image);

    // Update status
    labelStatus->setText(QString("<b>Processed live images:</b> %1. <b>Accepted:</b> %2").arg(imageCounter).arg(patternWorldPoints.size()));
}


cv::Mat PageSingleCameraDetection::getImageFromPipeline ()
{
    // Return left image
    return pipeline->getLeftImage();
}

void PageSingleCameraDetection::processImage ()
{
    // Make sure we aren't at the end already (only for non-live capture)
    if (!liveCapture && imageCounter >= images.size()) {
        // We are at the end of list; disable everything
        pushButtonAccept->setEnabled(false);
        pushButtonDiscard->setEnabled(false);
        pushButtonAuto->setEnabled(false);
        pushButtonAuto->setChecked(false);

        // Update status
        labelStatus->setText(QString("All %1 images processed. <b>Accepted:</b> %3.").arg(images.size()).arg(patternWorldPoints.size()));
    } else {
        // Update status
        if (!liveCapture) {
            QString currentFileBasename = QFileInfo(images[imageCounter]).fileName();
            labelStatus->setText(QString("<b>Image</b> %1 / %2. <b>Accepted</b> %3. <b>Current:</b> %4").arg(imageCounter).arg(images.size()).arg(patternWorldPoints.size()).arg(currentFileBasename));
        }

        // Enable both buttons
        pushButtonAccept->show();
        pushButtonAccept->setEnabled(true);

        pushButtonDiscard->show();
        pushButtonDiscard->setEnabled(true);

        // Clear the text on image display
        widgetImage->setText(QString());

        // Read image
        cv::Mat image;

        if (!liveCapture) {
            try {
                image = cv::imread(images[imageCounter].toStdString());
            } catch (cv::Exception &e) {
                QMessageBox::warning(this, "Image load", "Failed to load image: " + QString::fromStdString(e.what()));

                widgetImage->setText("Failed to load image!");
                widgetImage->setImage(cv::Mat());

                pushButtonAccept->hide();
                pushButtonAccept->setEnabled(false);
                return;
            }

            widgetImage->setImage(image);
        } else {
            // Disable live upodate and hide Process button
            disableLiveUpdate();

            image = getImageFromPipeline();
            widgetImage->setImage(image);
        }

        // Validate image size
        if (imageSize == cv::Size()) {
            // Store the first ...
            imageSize = image.size();
        } else if (image.size() != imageSize) {
            QMessageBox::warning(this, "Invalid size", "Image size does not match the size of first image!");
            pushButtonAccept->hide();
            pushButtonAccept->setEnabled(false);
            return;
        }

        // Find pattern
        std::vector<cv::Point2f> points;
        patternFound = calibrationPattern->findInImage(image, currentImagePoints);

        if (!patternFound) {
            pushButtonAccept->hide();
            pushButtonAccept->setEnabled(false);
        }

        widgetImage->setPattern(patternFound, currentImagePoints, calibrationPattern->getPatternSize());
    }

    // Auto process
    if (autoProcess) {
        autoProcessTimer->start(500);
    }
}


// *********************************************************************
// *                Pattern detection page: left camera                *
// *********************************************************************
PageLeftCameraDetection::PageLeftCameraDetection (Pipeline::Pipeline *pipeline, QWidget *parent)
    : PageSingleCameraDetection("LeftCamera", pipeline, parent)
{
    setTitle("Left camera calibration");
}

PageLeftCameraDetection::~PageLeftCameraDetection ()
{
}

int PageLeftCameraDetection::nextId () const
{
    return Wizard::PageId::LeftCameraCalibrationId;
}


// *********************************************************************
// *                Pattern detection page: right camera               *
// *********************************************************************
PageRightCameraDetection::PageRightCameraDetection (Pipeline::Pipeline *pipeline, QWidget *parent)
    : PageSingleCameraDetection("RightCamera", pipeline, parent)
{
    setTitle("Right camera calibration");
}

PageRightCameraDetection::~PageRightCameraDetection ()
{
}


int PageRightCameraDetection::nextId () const
{
    return Wizard::PageId::RightCameraCalibrationId;
}

cv::Mat PageRightCameraDetection::getImageFromPipeline ()
{
    // Return right image
    return pipeline->getRightImage();
}


// *********************************************************************
// *                   Pattern detection page: stereo                  *
// *********************************************************************
PageStereoDetection::PageStereoDetection (Pipeline::Pipeline *pipeline, QWidget *parent)
    : PageDetection("Stereo", pipeline, parent)
{
    setTitle("Stereo calibration");

    labelCaption->setText(
        "The procedure will try to locate calibration pattern in each "
        "calibration image pair, displaying result at each step. Please accept "
        "or discard the result (for example, if the pattern is mislocated). "
        "Please note that to complete this step, the pattern must be located "
        "in at least six image pairs."
    );

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());

    // Display widgets
    QHBoxLayout *hbox = new QHBoxLayout();
    layout->insertLayout(2, hbox);

    widgetImageLeft = new Widgets::CalibrationPatternDisplayWidget("Invalid image!", this);
    widgetImageLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hbox->addWidget(widgetImageLeft);

    widgetImageRight = new Widgets::CalibrationPatternDisplayWidget("Invalid image!", this);
    widgetImageRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hbox->addWidget(widgetImageRight);
}

PageStereoDetection::~PageStereoDetection ()
{
}


int PageStereoDetection::nextId () const
{
    return Wizard::PageId::StereoCalibrationId;
}


void PageStereoDetection::initializePage ()
{
    PageDetection::initializePage();

    // Clear images
    widgetImageLeft->setImage(cv::Mat());
    widgetImageRight->setImage(cv::Mat());
}


void PageStereoDetection::acceptPattern ()
{
    // Append coordinate vectors for both left and right image
    patternImagePoints.push_back(currentImagePointsLeft);
    patternImagePoints.push_back(currentImagePointsRight);

    // Append world coordinate vector
    patternWorldPoints.push_back(calibrationPattern->computePlanarCoordinates());

    imageCounter += 2;

    if (!liveCapture) {
        // Process next pair
        processImage();
    } else {
        // Re-enable live update
        enableLiveUpdate();
    }

    // We might be ready to go on...
    emit completeChanged();
}

void PageStereoDetection::discardPattern ()
{
    // Skip the pair
    imageCounter += 2;

    if (!liveCapture) {
        // Process next pair
        processImage();
    } else {
        // Re-enable live update
        enableLiveUpdate();
    }
}

void PageStereoDetection::updateLiveCapture ()
{
    if (!doLiveUpdate) {
        return;
    }

    // Display new images
    cv::Mat imageLeft, imageRight;
    pipeline->getImages(imageLeft, imageRight);

    widgetImageLeft->setImage(imageLeft);
    widgetImageRight->setImage(imageRight);

    // Update status
    labelStatus->setText(QString("<b>Processed live pairs:</b> %1. <b>Accepted:</b> %2").arg(imageCounter/2).arg(patternWorldPoints.size()));
}


void PageStereoDetection::processImage ()
{
    // Make sure we aren't at the end already (only for non-live capture)
    if (!liveCapture && imageCounter >= images.size()) {
        // We are at the end of list; disable everything
        pushButtonAccept->setEnabled(false);
        pushButtonDiscard->setEnabled(false);
        pushButtonAuto->setEnabled(false);
        pushButtonAuto->setChecked(false);

        // Update status
        labelStatus->setText(QString("All %1 pairs processed. <b>Accepted:</b> %3.").arg(images.size()/2).arg(patternWorldPoints.size()));
    } else {
        // Update status
        if (!liveCapture) {
            QString currentFileBasenameLeft = QFileInfo(images[imageCounter]).fileName();
            QString currentFileBasenameRight = QFileInfo(images[imageCounter+1]).fileName();
            labelStatus->setText(QString("<b>Pair</b> %1 / %2. <b>Accepted:</b> %3. <b>Current:</b> %4 | %5").arg(imageCounter/2).arg(images.size()/2).arg(patternWorldPoints.size()).arg(currentFileBasenameLeft).arg(currentFileBasenameRight));
        }

        // Enable both buttons
        pushButtonAccept->show();
        pushButtonAccept->setEnabled(true);

        pushButtonDiscard->show();
        pushButtonDiscard->setEnabled(true);

        // Clear the text on image display
        widgetImageLeft->setText(QString());
        widgetImageRight->setText(QString());

        // Read and display images
        cv::Mat imageLeft, imageRight;

        if (!liveCapture) {
            try {
                imageLeft = cv::imread(images[imageCounter].toStdString());
            } catch (cv::Exception &e) {
                QMessageBox::warning(this, "Image load", "Failed to load left image: " + QString::fromStdString(e.what()));

                widgetImageLeft->setText("Failed to load image!");
                widgetImageLeft->setImage(cv::Mat());

                pushButtonAccept->hide();
                pushButtonAccept->setEnabled(false);
                return;
            }

            widgetImageLeft->setImage(imageLeft);

            try {
                imageRight = cv::imread(images[imageCounter+1].toStdString());
            } catch (cv::Exception &e) {
                QMessageBox::warning(this, "Image load", "Failed to load right image: " + QString::fromStdString(e.what()));

                widgetImageRight->setText("Failed to right image!");
                widgetImageRight->setImage(cv::Mat());

                pushButtonAccept->hide();
                pushButtonAccept->setEnabled(false);
                return;
            }

            widgetImageRight->setImage(imageRight);
        } else {
            // Disable live upodate and hide Process button
            disableLiveUpdate();

            pipeline->getImages(imageLeft, imageRight);
            widgetImageLeft->setImage(imageLeft);
            widgetImageRight->setImage(imageRight);
        }


        // Validate image size
        if (imageSize == cv::Size()) {
            // Store the first ...
            imageSize = imageLeft.size();
        }

        if (imageLeft.size() != imageSize) {
            QMessageBox::warning(this, "Invalid size", "Left image size does not match the size of first image!");
            pushButtonAccept->hide();
            pushButtonAccept->setEnabled(false);
            return;
        }

        if (imageRight.size() != imageSize) {
            QMessageBox::warning(this, "Invalid size", "Right image size does not match the size of first image!");
            pushButtonAccept->hide();
            pushButtonAccept->setEnabled(false);
            return;
        }

        // Find pattern
        bool patternFoundLeft = calibrationPattern->findInImage(imageLeft, currentImagePointsLeft);
        widgetImageLeft->setPattern(patternFoundLeft, currentImagePointsLeft, calibrationPattern->getPatternSize());

        bool patternFoundRight = calibrationPattern->findInImage(imageRight, currentImagePointsRight);
        widgetImageRight->setPattern(patternFoundRight, currentImagePointsRight, calibrationPattern->getPatternSize());

        patternFound = patternFoundLeft && patternFoundRight;

        if (!patternFound) {
            pushButtonAccept->hide();
            pushButtonAccept->setEnabled(false);
        }
    }

    // Auto process
    if (autoProcess) {
        autoProcessTimer->start(500);
    }
}


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL
