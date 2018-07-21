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
PageDetection::PageDetection (const QString &fieldPrefix, Pipeline::Pipeline *pipeline, QWidget *parent)
    : QWizardPage(parent),
      fieldPrefix(fieldPrefix),
      autoProcess(false),
      autoProcessTimer(new QTimer(this)),
      calibrationPattern(new Pipeline::CalibrationPattern()),
      liveCaptureMode(false),
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
    liveCaptureMode = field(fieldPrefix + "LiveCapture").toBool();

    // Get fresh images list and reset the counter
    images = field(fieldPrefix + "Images").toStringList();
    imageCounter = 0;

    patternFound = false;

    // NOTE: images are cleared by children

    // Clear points
    patternImagePoints.clear();
    patternWorldPoints.clear();

    // Clear status label
    labelStatus->setText("Press \"Start\" to begin.");

    // Hide Accept and Discard buttons at first
    pushButtonAccept->hide();
    pushButtonDiscard->hide();

    // Show start button
    pushButtonStart->show();

    // Show Process button (live capture); or Start and
    // Auto buttons (non-live)
    if (liveCaptureMode) {
        pushButtonAuto->hide();

        pushButtonProcess->hide(); // Hide for now, but enable
    } else {
        pushButtonAuto->show();

        pushButtonProcess->hide();
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
    // This is an abuse of the ::validatePage() functionality, but we
    // have no other way to perform this sort of a cleanup when user
    // clicks on the Next button...

    if (liveCaptureMode) {
        // Disable live image update to reduce overhead
        doLiveUpdate = false;

        // Hide all other buttons, show Start button again, so that
        // user may re-start the capture
        pushButtonAccept->hide();
        pushButtonDiscard->hide();
        pushButtonProcess->hide();

        pushButtonStart->show();
        labelStatus->setText("Press \"Start\" to continue capture.");
    } else {
        // Stop the auto-processing
        pushButtonAuto->setChecked(false);
    }

    return QWizardPage::validatePage();
}

void PageDetection::cleanupPage ()
{
    // Disable live update on the page when user presses Back button
    // to avoid overhead of capturing and refreshing the image from
    // the image source
    doLiveUpdate = false;

    return QWizardPage::cleanupPage();
}


void PageDetection::startProcessing ()
{
    // Hide start button
    pushButtonStart->hide();

    if (liveCaptureMode) {
        // Enable live capture flag for live capture
        enableLiveUpdate();
    } else {
        // Process next
        processImage();
    }
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

PageSingleCameraDetection::PageSingleCameraDetection (const QString &fieldPrefix, Pipeline::Pipeline *pipeline, QWidget *parent)
    : PageDetection(fieldPrefix, pipeline, parent)
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

    // Register field: last processed image
    registerField(fieldPrefix + "LastProcessedImage", this, "lastProcessedImage");
}

PageSingleCameraDetection::~PageSingleCameraDetection ()
{
}

cv::Mat PageSingleCameraDetection::getLastProcessedImage () const
{
    return currentImage;
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

    if (!liveCaptureMode) {
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

    if (!liveCaptureMode) {
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
    if (!liveCaptureMode && imageCounter >= images.size()) {
        // We are at the end of list; disable everything
        pushButtonAuto->setChecked(false);
        pushButtonAuto->hide();

        pushButtonAccept->hide();
        pushButtonDiscard->hide();

        // Update status
        labelStatus->setText(QString("All %1 images processed. <b>Accepted:</b> %3.").arg(images.size()).arg(patternWorldPoints.size()));
    } else {
        // Update status
        if (!liveCaptureMode) {
            QString currentFileBasename = QFileInfo(images[imageCounter]).fileName();
            labelStatus->setText(QString("<b>Image</b> %1 / %2. <b>Accepted</b> %3. <b>Current:</b> %4").arg(imageCounter).arg(images.size()).arg(patternWorldPoints.size()).arg(currentFileBasename));
        }

        // Enable both buttons
        pushButtonAccept->show();
        pushButtonDiscard->show();

        // Clear the text on image display
        widgetImage->setText(QString());

        // Read image
        if (!liveCaptureMode) {
            try {
                currentImage = cv::imread(images[imageCounter].toStdString());
            } catch (cv::Exception &e) {
                QMessageBox::warning(this, "Image load", "Failed to load image: " + QString::fromStdString(e.what()));

                widgetImage->setText("Failed to load image!");
                widgetImage->setImage(cv::Mat());

                pushButtonAccept->hide();
                return;
            }

            widgetImage->setImage(currentImage);
        } else {
            // Disable live upodate and hide Process button
            disableLiveUpdate();

            currentImage = getImageFromPipeline();
            widgetImage->setImage(currentImage);
        }

        // Validate image size
        if (imageSize == cv::Size()) {
            // Store the first ...
            imageSize = currentImage.size();
        } else if (currentImage.size() != imageSize) {
            QMessageBox::warning(this, "Invalid size", "Image size does not match the size of first image!");
            pushButtonAccept->hide();
            return;
        }

        // Find pattern
        std::vector<cv::Point2f> points;
        patternFound = calibrationPattern->findInImage(currentImage, currentImagePoints);

        if (!patternFound) {
            pushButtonAccept->hide();
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

    // Register field: last processed image
    registerField(fieldPrefix + "LastProcessedImageLeft", this, "lastProcessedImageLeft");
    registerField(fieldPrefix + "LastProcessedImageRight", this, "lastProcessedImageRight");
}

PageStereoDetection::~PageStereoDetection ()
{
}


cv::Mat PageStereoDetection::getLastProcessedImageLeft () const
{
    return currentImageLeft;
}

cv::Mat PageStereoDetection::getLastProcessedImageRight () const
{
    return currentImageRight;
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

    if (!liveCaptureMode) {
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

    if (!liveCaptureMode) {
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
    if (!liveCaptureMode && imageCounter >= images.size()) {
        // We are at the end of list; disable everything
        pushButtonAuto->setChecked(false);
        pushButtonAuto->hide();

        pushButtonAccept->hide();
        pushButtonDiscard->hide();

        // Update status
        labelStatus->setText(QString("All %1 pairs processed. <b>Accepted:</b> %3.").arg(images.size()/2).arg(patternWorldPoints.size()));
    } else {
        // Update status
        if (!liveCaptureMode) {
            QString currentFileBasenameLeft = QFileInfo(images[imageCounter]).fileName();
            QString currentFileBasenameRight = QFileInfo(images[imageCounter+1]).fileName();
            labelStatus->setText(QString("<b>Pair</b> %1 / %2. <b>Accepted:</b> %3. <b>Current:</b> %4 | %5").arg(imageCounter/2).arg(images.size()/2).arg(patternWorldPoints.size()).arg(currentFileBasenameLeft).arg(currentFileBasenameRight));
        }

        // Enable both buttons
        pushButtonAccept->show();
        pushButtonDiscard->show();

        // Clear the text on image display
        widgetImageLeft->setText(QString());
        widgetImageRight->setText(QString());

        // Read and display images
        if (!liveCaptureMode) {
            try {
                currentImageLeft = cv::imread(images[imageCounter].toStdString());
            } catch (cv::Exception &e) {
                QMessageBox::warning(this, "Image load", "Failed to load left image: " + QString::fromStdString(e.what()));

                widgetImageLeft->setText("Failed to load image!");
                widgetImageLeft->setImage(cv::Mat());

                pushButtonAccept->hide();
                return;
            }
            widgetImageLeft->setImage(currentImageLeft);

            try {
                currentImageRight = cv::imread(images[imageCounter+1].toStdString());
            } catch (cv::Exception &e) {
                QMessageBox::warning(this, "Image load", "Failed to load right image: " + QString::fromStdString(e.what()));

                widgetImageRight->setText("Failed to right image!");
                widgetImageRight->setImage(cv::Mat());

                pushButtonAccept->hide();
                return;
            }
            widgetImageRight->setImage(currentImageRight);
        } else {
            // Disable live upodate and hide Process button
            disableLiveUpdate();

            pipeline->getImages(currentImageLeft, currentImageRight);
            widgetImageLeft->setImage(currentImageLeft);
            widgetImageRight->setImage(currentImageRight);
        }


        // Validate image size
        if (imageSize == cv::Size()) {
            // Store the first ...
            imageSize = currentImageLeft.size();
        }

        if (currentImageLeft.size() != imageSize) {
            QMessageBox::warning(this, "Invalid size", "Left image size does not match the size of first image!");
            pushButtonAccept->hide();
            return;
        }

        if (currentImageRight.size() != imageSize) {
            QMessageBox::warning(this, "Invalid size", "Right image size does not match the size of first image!");
            pushButtonAccept->hide();
            return;
        }

        // Find pattern
        bool patternFoundLeft = calibrationPattern->findInImage(currentImageLeft, currentImagePointsLeft);
        widgetImageLeft->setPattern(patternFoundLeft, currentImagePointsLeft, calibrationPattern->getPatternSize());

        bool patternFoundRight = calibrationPattern->findInImage(currentImageRight, currentImagePointsRight);
        widgetImageRight->setPattern(patternFoundRight, currentImagePointsRight, calibrationPattern->getPatternSize());

        patternFound = patternFoundLeft && patternFoundRight;

        if (!patternFound) {
            pushButtonAccept->hide();
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
