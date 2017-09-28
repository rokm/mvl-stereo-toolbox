/*
 * MVL Stereo Toolbox: calibration wizard: pattern detection page
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

#include "page_detection.h"
#include "wizard.h"

#include <stereo-pipeline/calibration_pattern.h>
#include <stereo-widgets/calibration_pattern_display_widget.h>

#include <opencv2/imgcodecs.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                   Pattern detection page: common                  *
// *********************************************************************
PageDetection::PageDetection (const QString &fieldPrefixString, bool pairs, QWidget *parent)
    : QWizardPage(parent),
      fieldPrefix(fieldPrefixString),
      processImagePairs(pairs),
      calibrationPattern(new Pipeline::CalibrationPattern())
{
    setSubTitle("Calibration pattern detection");

    QLabel *label;
    QFrame *separator;

    // Layout
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("The procedure will try to locate calibration pattern in each "
                       "calibration image, displaying result at each step. Please accept "
                       "or discard the result (for example, if the pattern is mislocated). "
                       "Please note that to complete this step, the pattern must be located "
                       "in at least six images (for individual cameras) or image pairs (for "
                       "stereo).");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label, 0, 0, 1, 2);

    // Separator
    separator = new QFrame(this);
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(separator, 1, 0, 1, 2);

    // Display
    widgetImage = new Widgets::CalibrationPatternDisplayWidget("Invalid image!", this);
    widgetImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(widgetImage, 2, 0, 1, 2);

    // Status label
    labelStatus = new QLabel(this);
    layout->addWidget(labelStatus, 3, 0, 1, 1);

    // Button box
    QHBoxLayout *buttonBox = new QHBoxLayout();
    layout->addLayout(buttonBox, 3, 1, 1, 1);

    buttonBox->addStretch();

    pushButtonAuto = new QPushButton("Auto", this);
    pushButtonAuto->setCheckable(true);
    connect(pushButtonAuto, &QPushButton::toggled, this, [this] (bool enable) {
        // Enable/disable auto processing
        autoProcess = enable;

        if (enable) {
            if (imageCounter == -1) {
                // Start processing
                startProcessing();
            } else {
                // Automatically process current frame
                doAutomaticProcessing();
            }
        }
    });
    buttonBox->addWidget(pushButtonAuto);

    pushButtonDiscard = new QPushButton("Discard", this);
    connect(pushButtonDiscard, &QPushButton::clicked, this, [this] () {
        // Disable auto processing
        pushButtonAuto->setChecked(false);

        // Manual discard
        discardPattern();
    });
    buttonBox->addWidget(pushButtonDiscard);

    pushButtonAccept = new QPushButton("Accept", this);
    connect(pushButtonAccept, &QPushButton::clicked, this, [this] () {
        // Disable auto processing
        pushButtonAuto->setChecked(false);

        // Manual accept
        acceptPattern();
    });
    buttonBox->addWidget(pushButtonAccept);

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
    // Get fresh images list and reset the counter
    images = field(fieldPrefix + "Images").toStringList();
    imageCounter = -1;

    // Clear image
    widgetImage->setImage(cv::Mat());
    widgetImage->setText("Press \"Start\" to begin");

    // Clear points
    patternImagePoints.clear();
    patternWorldPoints.clear();

    // Clear status label
    labelStatus->setText(QString());

    // Make "Accept" button a start one...
    pushButtonAccept->setText("Start");
    pushButtonAccept->show();
    pushButtonAccept->setEnabled(true);

    patternFound = false;
    pushButtonAuto->setChecked(false);

    // ... and disable "Discard" button
    pushButtonDiscard->setEnabled(false);

    // Enable "auto" button
    pushButtonAuto->setEnabled(true);

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
    pushButtonAccept->setText("Accept");
    pushButtonAccept->setEnabled(true);
    pushButtonDiscard->setEnabled(true);

    imageCounter = 0;
    patternFound = false;
    imageSize = cv::Size(); // Reset size

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

void PageDetection::acceptPattern ()
{
    if (imageCounter == -1) {
        // Start processing
        startProcessing();
        return;
    }

    // Accept
    if (processImagePairs) {
        // Always append image coordinates vector; for odd counter
        // numbers (second) images this is always safe, since if
        // first image was discarded, we would be skipping the second
        // one automatically. If first image is accepted, and second
        // is rejected, then the first image's points will have to
        // be removed from the list by the discard function.
        patternImagePoints.push_back(currentImagePoints);

        // For pairs, we append world coordinates vector only on odd
        // counter numbers (so when second image of the pair is accepted)
        if (imageCounter % 2) {
            patternWorldPoints.push_back(calibrationPattern->computePlanarCoordinates());
        }
    } else {
        // Append image coordinates
        patternImagePoints.push_back(currentImagePoints);
        // Append world coordinates
        patternWorldPoints.push_back(calibrationPattern->computePlanarCoordinates());
    }

    // Process next
    imageCounter++;
    processImage();
}

void PageDetection::discardPattern ()
{
    if (processImagePairs) {
        if (imageCounter % 2) {
            // Discarding second image; since we are here (see below),
            // this means that first image was accepted and we need to
            // remove its image coordinates from the list.
            patternImagePoints.pop_back();

            // Skip the image
            imageCounter++;
        } else {
            // Discarding first image; skip it together with the second
            // one...
            imageCounter += 2;
        }
    } else {
        // Just skip the image
        imageCounter++;
    }

    // Process next
    processImage();
}

void PageDetection::processImage ()
{
    // Make sure we aren't at the end already
    if (imageCounter >= images.size()) {
        // We are at the end of list; disable everything
        pushButtonAccept->setEnabled(false);
        pushButtonDiscard->setEnabled(false);
        pushButtonAuto->setEnabled(false);
        pushButtonAuto->setChecked(false);

        // Update status
        if (processImagePairs) {
            labelStatus->setText(QString("All %1 pairs processed. <b>Accepted:</b> %3.").arg(images.size()/2).arg(patternWorldPoints.size()));
        } else {
            labelStatus->setText(QString("All %1 images processed. <b>Accepted:</b> %3.").arg(images.size()).arg(patternWorldPoints.size()));
        }
    } else {
        // Update status
        QString currentFileBasename = QFileInfo(images[imageCounter]).fileName();
        if (processImagePairs) {
            labelStatus->setText(QString("<b>Pair</b> %1 / %2. <b>Accepted:</b> %3. <b>Current:</b> %4").arg(imageCounter/2 + 1).arg(images.size()/2).arg(patternWorldPoints.size()).arg(currentFileBasename));
        } else {
            labelStatus->setText(QString("<b>Image</b> %1 / %2. <b>Accepted</b> %3. <b>Current:</b> %4").arg(imageCounter).arg(images.size()).arg(patternWorldPoints.size()).arg(currentFileBasename));
        }

        // Enable both buttons
        pushButtonAccept->show();
        pushButtonAccept->setEnabled(true);
        pushButtonDiscard->setEnabled(true);

        // Clear the text on image display
        widgetImage->setText(QString());

        // Read image
        cv::Mat image;
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

    // We might be ready to go on...
    emit completeChanged();

    // Auto process
    if (autoProcess) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
        QTimer::singleShot(500, this, &PageDetection::doAutomaticProcessing);
#else
        QTimer::singleShot(500, this, SLOT(doAutomaticProcessing()));
#endif
    }
}


// *********************************************************************
// *               Pattern detection page: single camera               *
// *********************************************************************
PageSingleCameraDetection::PageSingleCameraDetection (QWidget *parent)
    : PageDetection("SingleCamera", false, parent)
{
    setTitle("Single camera calibration");
}

PageSingleCameraDetection::~PageSingleCameraDetection ()
{
}

int PageSingleCameraDetection::nextId () const
{
    return Wizard::PageId::SingleCameraCalibrationId;
}


// *********************************************************************
// *                Pattern detection page: left camera                *
// *********************************************************************
PageLeftCameraDetection::PageLeftCameraDetection (QWidget *parent)
    : PageDetection("LeftCamera", false, parent)
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
PageRightCameraDetection::PageRightCameraDetection (QWidget *parent)
    : PageDetection("RightCamera", false, parent)
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



// *********************************************************************
// *                   Pattern detection page: stereo                  *
// *********************************************************************
PageStereoDetection::PageStereoDetection (QWidget *parent)
    : PageDetection("Stereo", true, parent)
{
    setTitle("Stereo calibration");
}

PageStereoDetection::~PageStereoDetection ()
{
}

int PageStereoDetection::nextId () const
{
    return Wizard::PageId::StereoCalibrationId;
}


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL
