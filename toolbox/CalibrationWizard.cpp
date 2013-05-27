/*
 * MVL Stereo Toolbox: calibration wizard
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

#include "CalibrationWizard.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "CalibrationPatternDisplayWidget.h"
#include "ImagePairDisplayWidget.h"

// *********************************************************************
// *                               Wizard                              *
// *********************************************************************
CalibrationWizard::CalibrationWizard (QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle("Calibration Wizard");
    setWizardStyle(ModernStyle);

    setPixmap(QWizard::LogoPixmap, QPixmap(":/images/calibration.png"));

    // Register metatypes for OpenCV types that are passed between pages
    qRegisterMetaType< std::vector<std::vector<cv::Point2f> > >();
    qRegisterMetaType< std::vector<std::vector<cv::Point3f> > >();
    qRegisterMetaType< cv::Size >();
    qRegisterMetaType< cv::Mat >();

    // Pages
    setPage(PageIntroduction, new CalibrationWizardPageIntroduction(this));
    setPage(PageStereoImages, new CalibrationWizardPageStereoImages(this));
    setPage(PageStereoDetection, new CalibrationWizardPageStereoDetection(this));
    setPage(PageStereoCalibration, new CalibrationWizardPageStereoCalibration(this));
    setPage(PageStereoResult, new CalibrationWizardPageStereoResult(this));
    setPage(PageLeftCameraImages, new CalibrationWizardPageLeftCameraImages(this));
    setPage(PageLeftCameraDetection, new CalibrationWizardPageLeftCameraDetection(this));
    setPage(PageLeftCameraCalibration, new CalibrationWizardPageLeftCameraCalibration(this));
    setPage(PageLeftCameraResult, new CalibrationWizardPageLeftCameraResult(this));
    setPage(PageRightCameraImages, new CalibrationWizardPageRightCameraImages(this));
    setPage(PageRightCameraDetection, new CalibrationWizardPageRightCameraDetection(this));
    setPage(PageRightCameraCalibration, new CalibrationWizardPageRightCameraCalibration(this));
    setPage(PageRightCameraResult, new CalibrationWizardPageRightCameraResult(this));

    setStartId(PageIntroduction);
}

CalibrationWizard::~CalibrationWizard()
{
}


// *********************************************************************
// *                        Page: introduction                         *
// *********************************************************************
CalibrationWizardPageIntroduction::CalibrationWizardPageIntroduction (QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Introduction");

    QLabel *label;
    QFrame *separator;

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("This wizard will guide you through stereo calibration process.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label);

    // Separator
    separator = new QFrame(this);
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addWidget(separator);

    // Calibration mode
    label = new QLabel("Please select desired calibration mode below. In <i>joint calibration</i>, you will be "
                       "asked for a single sequence of image pairs, from which both cameras' intrinsic parameters "
                       "(camera matrix and distortion coefficients) and stereo parameters will be estimated. In <i>decoupled "
                       "calibration</i>, you will be first asked for one sequence of calibration images for each camera, and their "
                       "intrinsic parameters will be estimated separately. Then, you will be asked for a sequence of image pairs "
                       "from which stereo parameters will be estimated.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label);

    radioButtonJointCalibration = new QRadioButton("&Joint calibration");
    radioButtonJointCalibration->setToolTip("Estimate all parameters together.");
    layout->addWidget(radioButtonJointCalibration);

    radioButtonDecoupledCalibration = new QRadioButton("&Decoupled calibration");
    radioButtonDecoupledCalibration->setToolTip("Separately estimate intrinsic parameters and stereo parameters.");
    layout->addWidget(radioButtonDecoupledCalibration);

    registerField("JointCalibration", radioButtonJointCalibration);

    radioButtonJointCalibration->setChecked(true);
}

CalibrationWizardPageIntroduction::~CalibrationWizardPageIntroduction ()
{
}

int CalibrationWizardPageIntroduction::nextId () const
{
    if (radioButtonJointCalibration->isChecked()) {
        return CalibrationWizard::PageStereoImages;
    } else {
        return CalibrationWizard::PageLeftCameraImages;
    }
}


// *********************************************************************
// *               Page: images selection: common class                *
// *********************************************************************
CalibrationWizardPageImages::CalibrationWizardPageImages (const QString &fieldPrefixString, QWidget *parent)
    : QWizardPage(parent), fieldPrefix(fieldPrefixString)
{    
    setSubTitle("Calibration image sequence and pattern");

    QLabel *label;
    QFrame *separator;
    QGroupBox *box;

    // Layout
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Please select calibration images and provide information about calibration pattern. For "
                       "individual cameras, at least six images are required. For stereo, at least six image pairs "
                       "are required (number of images must be even).");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label, 0, 0, 1, 2);

    // *** Calibration images ***
    box = new QGroupBox("Calibration images", this);
    layout->addWidget(box, 1, 0, 1, 1);

    // Layout
    QGridLayout *imagesLayout = new QGridLayout(box);

    // Add images button
    pushButtonAddImages = new QPushButton("Add");
    pushButtonAddImages->setToolTip("Add images");
    connect(pushButtonAddImages, SIGNAL(released()), this, SLOT(addImages()));

    imagesLayout->addWidget(pushButtonAddImages, 0, 0, 1, 1);

    // Clear images button
    pushButtonClearImages = new QPushButton("Clear");
    pushButtonClearImages->setToolTip("Clear images");
    connect(pushButtonClearImages, SIGNAL(released()), this, SLOT(clearImages()));

    imagesLayout->addWidget(pushButtonClearImages, 0, 1, 1, 1);

    // Images list
    listWidgetImages = new QListWidget(this);

    imagesLayout->addWidget(listWidgetImages, 1, 0, 1, 2);


    // *** Calibration pattern ***
    box = new QGroupBox("Calibration pattern", this);
    layout->addWidget(box, 1, 1, 1, 1);
    
    // Layout
    QFormLayout *patternLayout = new QFormLayout(box);

    // Pattern width
    label = new QLabel("Pattern width", this);
    label->setToolTip("Pattern width:\n"
                      " - chessboard: number of inside corners in horizontal direction\n"
                      " - circle grid: number of circles in horizontal direction\n"
                      " - asymmetric circle grid: number of circles in first column (*vertical* direction!)");
    
    spinBoxPatternWidth = new QSpinBox(this);
    spinBoxPatternWidth->setRange(1, 1000);
    spinBoxPatternWidth->setValue(19);
    patternLayout->addRow(label, spinBoxPatternWidth);

    // Pattern height
    label = new QLabel("Pattern height", this);
    label->setToolTip("Pattern width:\n"
                      " - chessboard: number of inside corners in vertical direction\n"
                      " - circle grid: number of circles in vertical direction\n"
                      " - asymmetric circle grid: total number of circles in first *two* rows (*horizontal* direction!)");
    
    spinBoxPatternHeight = new QSpinBox(this);
    spinBoxPatternHeight->setRange(1, 1000);
    spinBoxPatternHeight->setValue(12);
    patternLayout->addRow(label, spinBoxPatternHeight);

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
    patternLayout->addRow(label, spinBoxElementSize);

    // Pattern type
    label = new QLabel("Pattern type", this);
    label->setToolTip("Calibration pattern type");

    comboBoxPatternType = new QComboBox(this);
    comboBoxPatternType->addItem("Chessboard", CalibrationPattern::Chessboard);
    comboBoxPatternType->addItem("Circle grid", CalibrationPattern::Circles);
    comboBoxPatternType->addItem("Asymmetric circle grid", CalibrationPattern::AsymmetricCircles);
    comboBoxPatternType->setCurrentIndex(0);
    patternLayout->addRow(label, comboBoxPatternType);

    // Separator
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    patternLayout->addRow(separator);

    // Max image scale level
    label = new QLabel("Image scale levels", this);
    label->setToolTip("Maximum image scale level. If pattern is not found at original image size,\n"
                      "image is upsampled and search is repeated. The image scale is 1.0 + level*scaleIncrement,\n"
                      "where level goes from 0 to imageScaleLevels. Set this variable to 0 to disable\n"
                      "multi-scale search.");

    spinBoxScaleLevels = new QSpinBox(this);
    spinBoxScaleLevels->setRange(0, 100);
    spinBoxScaleLevels->setValue(0);
    patternLayout->addRow(label, spinBoxScaleLevels);

    // Max image scale level
    label = new QLabel("Scale increment", this);
    label->setToolTip("Scale increment for multi-scale pattern search. For details, see description of \n"
                      "image scale levels.");

    spinBoxScaleIncrement = new QDoubleSpinBox(this);
    spinBoxScaleIncrement->setRange(0.0, 2.0);
    spinBoxScaleIncrement->setSingleStep(0.05);
    spinBoxScaleIncrement->setValue(0.25);
    patternLayout->addRow(label, spinBoxScaleIncrement);


    // Register fields...
    registerField(fieldPrefix + "Images", this, "images"); // This one does not have a valid "physical" widget (since QListWidget does not return string list)...

    registerField(fieldPrefix + "PatternWidth", spinBoxPatternWidth);
    registerField(fieldPrefix + "PatternHeight", spinBoxPatternHeight);
    registerField(fieldPrefix + "ElementSize", spinBoxElementSize, "value"); // QWizard does not know QDoubleSpinBox!
    registerField(fieldPrefix + "PatternType", this, "patternType"); // Because default implementation returns selected index, not value
    registerField(fieldPrefix + "ScaleLevels", spinBoxScaleLevels);
    registerField(fieldPrefix + "ScaleIncrement", spinBoxScaleIncrement, "value");  // QWizard does not know QDoubleSpinBox!
}

CalibrationWizardPageImages::~CalibrationWizardPageImages ()
{
}

void CalibrationWizardPageImages::addImages ()
{
    // Get filename(s)
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select calibration images or list file", QString(), "Images (*.jpg *.png *.bmp *.tif *.ppm *.pgm);; Text file (*.txt)");

    if (!fileNames.size()) {
        emit completeChanged();
        return;
    }

    if (fileNames.size() == 1) {
        // Single file is given, assume it is a text file with a list
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
                QMessageBox::warning(this, "Image list error", "Image list is empty!");
                emit completeChanged();
                return;
            }
        } else {
            QMessageBox::warning(this, "Image list error", QString("Failed to open image list file \"%1\"!").arg(fileNames[0]));
            emit completeChanged();
            return;
        }
    }

    // Display them in the list
    foreach (const QString &fileName, fileNames) {
        QFileInfo info(fileName);

        QListWidgetItem *item = new QListWidgetItem(fileName, listWidgetImages);
        listWidgetImages->addItem(item);

        if (!info.isReadable()) {
            item->setForeground(Qt::red);
            item->setToolTip("File cannot be read!");
        }
    }
    
    // Revalidate
    emit completeChanged();
}

void CalibrationWizardPageImages::clearImages ()
{
    listWidgetImages->clear();
    emit completeChanged();
}

QStringList CalibrationWizardPageImages::getImages () const
{
    QStringList images;

    foreach (QListWidgetItem *item, listWidgetImages->findItems("*", Qt::MatchWrap | Qt::MatchWildcard)) {
        QFileInfo info(item->text());

        if (info.isReadable()) {
            images.append(item->text());
        }
    }

    return images;
}

int CalibrationWizardPageImages::getPatternType () const
{
    return comboBoxPatternType->itemData(comboBoxPatternType->currentIndex()).toInt();
}


// *********************************************************************
// *                   Page: images selection: stereo                  *
// *********************************************************************
CalibrationWizardPageStereoImages::CalibrationWizardPageStereoImages (QWidget *parent)
    : CalibrationWizardPageImages("Stereo", parent)
{
    setTitle("Stereo calibration");
}

CalibrationWizardPageStereoImages::~CalibrationWizardPageStereoImages ()
{
}

int CalibrationWizardPageStereoImages::nextId () const
{
    return CalibrationWizard::PageStereoDetection;
}

bool CalibrationWizardPageStereoImages::isComplete () const
{
    QStringList filenames = getImages();

    // Must be even number of images
    if (filenames.size() % 2) {
        return false;
    }

    // Must be at least six image pairs
    if (filenames.size() < 6*2) {
        return false;
    }

    return true;
}


// *********************************************************************
// *                Page: images selection: left camera                *
// *********************************************************************
CalibrationWizardPageLeftCameraImages::CalibrationWizardPageLeftCameraImages (QWidget *parent)
    : CalibrationWizardPageImages("LeftCamera", parent)
{
    setTitle("Left camera calibration");
}

CalibrationWizardPageLeftCameraImages::~CalibrationWizardPageLeftCameraImages ()
{
}

int CalibrationWizardPageLeftCameraImages::nextId () const
{
    return CalibrationWizard::PageLeftCameraDetection;
}

bool CalibrationWizardPageLeftCameraImages::isComplete () const
{
    QStringList filenames = getImages();

    // Must be at least six images
    if (filenames.size() < 6) {
        return false;
    }

    return true;
}


// *********************************************************************
// *                Page: images selection: right camera               *
// *********************************************************************
CalibrationWizardPageRightCameraImages::CalibrationWizardPageRightCameraImages (QWidget *parent)
    : CalibrationWizardPageImages("RightCamera", parent)
{
    setTitle("Right camera calibration");
}

CalibrationWizardPageRightCameraImages::~CalibrationWizardPageRightCameraImages ()
{
}

int CalibrationWizardPageRightCameraImages::nextId () const
{
    return CalibrationWizard::PageRightCameraDetection;
}

bool CalibrationWizardPageRightCameraImages::isComplete () const
{
    QStringList filenames = getImages();

    // Must be at least six images
    if (filenames.size() < 6) {
        return false;
    }

    return true;
}


// *********************************************************************
// *               Page: pattern detection: common class               *
// *********************************************************************
CalibrationWizardPageDetection::CalibrationWizardPageDetection (const QString &fieldPrefixString, bool pairs, QWidget *parent)
    : QWizardPage(parent), fieldPrefix(fieldPrefixString), processImagePairs(pairs)
{   
    setSubTitle("Calibration pattern detection");

    QLabel *label;
    QFrame *separator;
    QGroupBox *box;

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
    displayImage = new CalibrationPatternDisplayWidget("Invalid image!", this);
    displayImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayImage, 2, 0, 1, 2);

    // Status label
    labelStatus = new QLabel(this);
    layout->addWidget(labelStatus, 3, 0, 1, 1);

    // Button box
    buttonBox = new QDialogButtonBox(this);
    layout->addWidget(buttonBox, 3, 1, 1, 1);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptPattern()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(discardPattern()));

    pushButtonAccept = new QPushButton("Accept", this);
    buttonBox->addButton(pushButtonAccept, QDialogButtonBox::AcceptRole);

    pushButtonDiscard = new QPushButton("Discard", this);
    buttonBox->addButton(pushButtonDiscard, QDialogButtonBox::RejectRole);

    // Fields
    registerField(fieldPrefix + "PatternImagePoints", this, "patternImagePoints");
    registerField(fieldPrefix + "PatternWorldPoints", this, "patternWorldPoints");
    registerField(fieldPrefix + "ImageSize", this, "imageSize");
}

CalibrationWizardPageDetection::~CalibrationWizardPageDetection ()
{
}


const std::vector<std::vector<cv::Point2f> > &CalibrationWizardPageDetection::getPatternImagePoints () const
{
    return patternImagePoints;
}

const std::vector<std::vector<cv::Point3f> > &CalibrationWizardPageDetection::getPatternWorldPoints () const
{
    return patternWorldPoints;
}

const cv::Size &CalibrationWizardPageDetection::getImageSize () const
{
    return imageSize;
}


void CalibrationWizardPageDetection::initializePage ()
{
    // Get fresh images list and reset the counter
    images = field(fieldPrefix + "Images").toStringList();
    imageCounter = -1;

    // Clear image
    displayImage->setImage(cv::Mat());
    displayImage->setText("Press \"Start\" to begin");

    // Clear points
    patternImagePoints.clear();
    patternWorldPoints.clear();

    // Clear status label
    labelStatus->setText(QString());

    // Make "Accept" button a start one...
    pushButtonAccept->setText("Start");
    pushButtonAccept->show();
    pushButtonAccept->setEnabled(true);

    // ... and disable "Discard" button
    pushButtonDiscard->setEnabled(false);

    // Set parameters for pattern detector
    calibrationPattern.setParameters(
        field(fieldPrefix + "PatternWidth").toInt(),
        field(fieldPrefix + "PatternHeight").toInt(),
        field(fieldPrefix + "ElementSize").toDouble(),
        (CalibrationPattern::PatternType)field(fieldPrefix + "PatternType").toInt(),
        field(fieldPrefix + "ScaleLevels").toInt(),
        field(fieldPrefix + "ScaleIncrement").toDouble()
    ); 
}


bool CalibrationWizardPageDetection::isComplete () const
{
    // We need at least six valid images/pairs (= world coordinate vectors)
    return patternWorldPoints.size() >= 6;
}


void CalibrationWizardPageDetection::acceptPattern ()
{
    if (imageCounter == -1) {
        // Start
        pushButtonAccept->setText("Accept");
        pushButtonAccept->setEnabled(true);
        pushButtonDiscard->setEnabled(true);
        
        imageCounter++;
    } else {
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
                patternWorldPoints.push_back(calibrationPattern.computePlanarCoordinates());
            }
        } else {
            // Append image coordinates
            patternImagePoints.push_back(currentImagePoints);
            // Append world coordinates
            patternWorldPoints.push_back(calibrationPattern.computePlanarCoordinates());
        }
        
        imageCounter++;
    }

    // Process next
    processNextImage();
}

void CalibrationWizardPageDetection::discardPattern ()
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
    processNextImage();
}

void CalibrationWizardPageDetection::processNextImage ()
{
    // We might be ready to go on...
    emit completeChanged();

    // Make sure we aren't at the end already
    if (imageCounter >= images.size()) {
        // We are at the end of list; disable everything
        pushButtonAccept->setEnabled(false);
        pushButtonDiscard->setEnabled(false);

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
        displayImage->setText(QString());

        // Read image
        cv::Mat image;
        try {
            image = cv::imread(images[imageCounter].toStdString());
        } catch (cv::Exception e) {
            QMessageBox::warning(this, "Image load", "Failed to load image: " + QString::fromStdString(e.what()));

            displayImage->setText("Failed to load image!");
            displayImage->setImage(cv::Mat());
            
            pushButtonAccept->hide();
            pushButtonAccept->setEnabled(false);

            imageCounter++;
            return;
        }

        displayImage->setImage(image);

        // Validate image size
        if (imageSize == cv::Size()) {
            // Store the first ...
            imageSize = image.size();
        } else if (image.size() != imageSize) {
            QMessageBox::warning(this, "Invalid size", "Image size does not match the size of first image!");
            pushButtonAccept->hide();
            pushButtonAccept->setEnabled(false);

            imageCounter++;
            return;
        }

        // Find pattern
        std::vector<cv::Point2f> points;
        bool found = calibrationPattern.findInImage(image, currentImagePoints);

        if (!found) {
            pushButtonAccept->hide();
            pushButtonAccept->setEnabled(false);
        }

        displayImage->setPattern(found, currentImagePoints, calibrationPattern.getPatternSize());
    }
}


// *********************************************************************
// *                  Page: pattern detection: stereo                  *
// *********************************************************************
CalibrationWizardPageStereoDetection::CalibrationWizardPageStereoDetection (QWidget *parent)
    : CalibrationWizardPageDetection("Stereo", true, parent)
{
    setTitle("Stereo calibration");
}

CalibrationWizardPageStereoDetection::~CalibrationWizardPageStereoDetection ()
{
}

int CalibrationWizardPageStereoDetection::nextId () const
{
    return CalibrationWizard::PageStereoCalibration;
}

// *********************************************************************
// *               Page: pattern detection: left camera                *
// *********************************************************************
CalibrationWizardPageLeftCameraDetection::CalibrationWizardPageLeftCameraDetection (QWidget *parent)
    : CalibrationWizardPageDetection("LeftCamera", false, parent)
{
    setTitle("Left camera calibration");
}

CalibrationWizardPageLeftCameraDetection::~CalibrationWizardPageLeftCameraDetection ()
{
}

int CalibrationWizardPageLeftCameraDetection::nextId () const
{
    return CalibrationWizard::PageLeftCameraCalibration;
}

// *********************************************************************
// *                Page: pattern detection: right camera              *
// *********************************************************************
CalibrationWizardPageRightCameraDetection::CalibrationWizardPageRightCameraDetection (QWidget *parent)
    : CalibrationWizardPageDetection("RightCamera", false, parent)
{
    setTitle("Right camera calibration");
}

CalibrationWizardPageRightCameraDetection::~CalibrationWizardPageRightCameraDetection ()
{
}

int CalibrationWizardPageRightCameraDetection::nextId () const
{
    return CalibrationWizard::PageRightCameraCalibration;
}


// *********************************************************************
// *       Page: calibration: common class for left/right camera       *
// *********************************************************************
CalibrationWizardPageCalibration::CalibrationWizardPageCalibration (const QString &fieldPrefixString, QWidget *parent)
    : QWizardPage(parent), fieldPrefix(fieldPrefixString)
{
    setSubTitle("Calibration parameters");

    QLabel *label;

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Please enter parameters for calibration method, and press \"Calibrate\" to perform calibration. Depending on "
                       "number of images and hardware, the operation might take some time, during which the GUI will appear to be frozen.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label);

    // Fields
    registerField(fieldPrefix + "CameraMatrix", this, "cameraMatrix");
    registerField(fieldPrefix + "DistCoeffs", this, "distCoeffs");
}

CalibrationWizardPageCalibration::~CalibrationWizardPageCalibration ()
{
}

cv::Mat CalibrationWizardPageCalibration::getCameraMatrix () const
{
    return cameraMatrix;
}

cv::Mat CalibrationWizardPageCalibration::getDistCoeffs () const
{
    return distCoeffs;
}


void CalibrationWizardPageCalibration::initializePage ()
{
    oldNextButtonText = wizard()->buttonText(QWizard::NextButton);
    wizard()->setButtonText(QWizard::NextButton, "Calibrate");
}

void CalibrationWizardPageCalibration::cleanupPage ()
{
    wizard()->setButtonText(QWizard::NextButton, oldNextButtonText);
}

bool CalibrationWizardPageCalibration::validatePage ()
{
    std::vector<std::vector<cv::Point2f> > imagePoints = field(fieldPrefix + "PatternImagePoints").value< std::vector<std::vector<cv::Point2f> > >();
    std::vector<std::vector<cv::Point3f> > worldPoints = field(fieldPrefix + "PatternWorldPoints").value< std::vector<std::vector<cv::Point3f> > >();
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();
    
    // Perform calibration here - we use OpenCV directly and bypass
    // the pipeline's stereo calibration function, so that calibration
    // can be cancelled at any point without affecting the pipeline...

    int flags = 0;
    double err;

    cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    distCoeffs = cv::Mat();
   
    try {
        err = cv::calibrateCamera(worldPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, cv::noArray(), cv::noArray(), flags);
    } catch (cv::Exception e) {
        QMessageBox::warning(this, "Error", "Calibration failed: " + QString::fromStdString(e.what()));
        return false;
    }

    wizard()->setButtonText(QWizard::NextButton, oldNextButtonText);
    return true;
}



// *********************************************************************
// *                   Page: calibration: left camera                  *
// *********************************************************************
CalibrationWizardPageLeftCameraCalibration::CalibrationWizardPageLeftCameraCalibration (QWidget *parent)
    : CalibrationWizardPageCalibration("LeftCamera", parent)
{
    setTitle("Left camera calibration");
}

CalibrationWizardPageLeftCameraCalibration::~CalibrationWizardPageLeftCameraCalibration ()
{
}

int CalibrationWizardPageLeftCameraCalibration::nextId () const
{
    return CalibrationWizard::PageLeftCameraResult;
}

// *********************************************************************
// *                  Page: calibration: right camera                  *
// *********************************************************************
CalibrationWizardPageRightCameraCalibration::CalibrationWizardPageRightCameraCalibration (QWidget *parent)
    : CalibrationWizardPageCalibration("RightCamera", parent)
{
    setTitle("Right camera calibration");
}

CalibrationWizardPageRightCameraCalibration::~CalibrationWizardPageRightCameraCalibration ()
{
}

int CalibrationWizardPageRightCameraCalibration::nextId () const
{
    return CalibrationWizard::PageRightCameraResult;
}

// *********************************************************************
// *                     Page: calibration: stereo                     *
// *********************************************************************
CalibrationWizardPageStereoCalibration::CalibrationWizardPageStereoCalibration (QWidget *parent)
    : QWizardPage(parent), fieldPrefix("Stereo")
{
    setSubTitle("Calibration parameters");

    QLabel *label;

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Please enter parameters for calibration method, and press \"Calibrate\" to perform calibration. Depending on "
                       "number of images and hardware, the operation might take some time, during which the GUI will appear to be frozen.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label);

    // Fields
    registerField(fieldPrefix + "CameraMatrix1", this, "cameraMatrix1");
    registerField(fieldPrefix + "DistCoeffs1", this, "distCoeffs1");
    registerField(fieldPrefix + "CameraMatrix2", this, "cameraMatrix2");
    registerField(fieldPrefix + "DistCoeffs2", this, "distCoeffs2");
    registerField(fieldPrefix + "R", this, "R");
    registerField(fieldPrefix + "T", this, "T");
}

CalibrationWizardPageStereoCalibration::~CalibrationWizardPageStereoCalibration ()
{
}


const cv::Mat &CalibrationWizardPageStereoCalibration::getCameraMatrix1 () const
{
    return cameraMatrix1;
}

const cv::Mat &CalibrationWizardPageStereoCalibration::getDistCoeffs1 () const
{
    return distCoeffs1;
}

const cv::Mat &CalibrationWizardPageStereoCalibration::getCameraMatrix2 () const
{
    return cameraMatrix2;
}

const cv::Mat &CalibrationWizardPageStereoCalibration::getDistCoeffs2 () const
{
    return distCoeffs2;
}

const cv::Mat &CalibrationWizardPageStereoCalibration::getR () const
{
    return R;
}

const cv::Mat &CalibrationWizardPageStereoCalibration::getT () const
{
    return T;
}

const cv::Size &CalibrationWizardPageStereoCalibration::getImageSize () const
{
    return imageSize;
}


void CalibrationWizardPageStereoCalibration::initializePage ()
{
    oldNextButtonText = wizard()->buttonText(QWizard::NextButton);
    wizard()->setButtonText(QWizard::NextButton, "Calibrate");
}

void CalibrationWizardPageStereoCalibration::cleanupPage ()
{
    wizard()->setButtonText(QWizard::NextButton, oldNextButtonText);
}

bool CalibrationWizardPageStereoCalibration::validatePage ()
{
    std::vector<std::vector<cv::Point2f> > imagePoints = field(fieldPrefix + "PatternImagePoints").value< std::vector<std::vector<cv::Point2f> > >();
    std::vector<std::vector<cv::Point3f> > objectPoints = field(fieldPrefix + "PatternWorldPoints").value< std::vector<std::vector<cv::Point3f> > >();
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();
    
    // Perform calibration here - we use OpenCV directly and bypass
    // the pipeline's stereo calibration function, so that calibration
    // can be cancelled at any point without affecting the pipeline...

    // We need to split image points in two
    std::vector<std::vector<cv::Point2f> > imagePoints1;
    std::vector<std::vector<cv::Point2f> > imagePoints2;

    for (int i = 0; i < imagePoints.size(); i += 2) {
        imagePoints1.push_back(imagePoints[i]);
        imagePoints2.push_back(imagePoints[i+1]);
    }

    qDebug() << "Calibration: got" << objectPoints.size() << imagePoints.size() << "WORLD:" << imagePoints1.size() << imagePoints2.size();

    int flags = cv::CALIB_RATIONAL_MODEL;
    double err;

    if (field("JointCalibration").toBool()) {
        // Join calibration
        qDebug() << "Calibrating all";
        
        cameraMatrix1 = cv::Mat::eye(3, 3, CV_64F);
        distCoeffs1 = cv::Mat();

        cameraMatrix2 = cv::Mat::eye(3, 3, CV_64F);
        distCoeffs2 = cv::Mat();
    } else {
        qDebug() << "Fixing intrinsics";
        
        // Decoupled calibration, which means we already have camera
        // matrices and distortion coefficients
        cameraMatrix1 = field("LeftCameraCameraMatrix").value<cv::Mat>();
        distCoeffs1 = field("LeftCameraDistCoeffs").value<cv::Mat>();
        cameraMatrix2 = field("RightCameraCameraMatrix").value<cv::Mat>();
        distCoeffs2 = field("RightCameraDistCoeffs").value<cv::Mat>();

        flags |= cv::CALIB_FIX_INTRINSIC; // Estimate only R and T
    }
   
    try {
        err = cv::stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
                                  cameraMatrix1, distCoeffs1,
                                  cameraMatrix2, distCoeffs2,
                                  imageSize, R, T, cv::noArray(), cv::noArray(),
                                  cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-6),
                                  flags);
    } catch (cv::Exception e) {
        QMessageBox::warning(this, "Error", "Calibration failed: " + QString::fromStdString(e.what()));
        return false;
    }

    qDebug() << "RMS:" << err;

    wizard()->setButtonText(QWizard::NextButton, oldNextButtonText);
    return true;
}




// *********************************************************************
// *                    Page: result: common class                     *
// *********************************************************************
CalibrationWizardPageResult::CalibrationWizardPageResult (const QString &fieldPrefixString, QWidget *parent)
    : QWizardPage(parent), fieldPrefix(fieldPrefixString)
{
    setSubTitle("Calibration result");

    QLabel *label;

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Congratulations! Camera's intrinsic parameters have been calibrated. One of calibration images has been undistorted and is shown below for visual validation of calibration result.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label);

    // Undistorted image
    displayImage = new ImageDisplayWidget("Undistorted image", this);
    displayImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayImage);
}

CalibrationWizardPageResult::~CalibrationWizardPageResult ()
{
}

void CalibrationWizardPageResult::initializePage ()
{
    // Get camera calibration
    cv::Mat M = field(fieldPrefix + "CameraMatrix").value<cv::Mat>();
    cv::Mat D = field(fieldPrefix + "DistCoeffs").value<cv::Mat>();

    // Load and undistort first image
    QStringList images = field(fieldPrefix + "Images").toStringList();
    
    cv::Mat image = cv::imread(images[0].toStdString(), -1);
    cv::Mat undistortedImage;
    
    cv::undistort(image, undistortedImage, M, D);

    // Display undistorted image
    displayImage->setImage(undistortedImage);
}


// *********************************************************************
// *                    Page: result: left camera                      *
// *********************************************************************
CalibrationWizardPageLeftCameraResult::CalibrationWizardPageLeftCameraResult (QWidget *parent)
    : CalibrationWizardPageResult("LeftCamera", parent)
{
    setTitle("Left camera calibration");
}

CalibrationWizardPageLeftCameraResult::~CalibrationWizardPageLeftCameraResult ()
{
}

int CalibrationWizardPageLeftCameraResult::nextId () const
{
    return CalibrationWizard::PageRightCameraImages;
}

// *********************************************************************
// *                   Page: result: right camera                      *
// *********************************************************************
CalibrationWizardPageRightCameraResult::CalibrationWizardPageRightCameraResult (QWidget *parent)
    : CalibrationWizardPageResult("RightCamera", parent)
{
    setTitle("Right camera calibration");
}

CalibrationWizardPageRightCameraResult::~CalibrationWizardPageRightCameraResult ()
{
}

int CalibrationWizardPageRightCameraResult::nextId () const
{
    return CalibrationWizard::PageStereoImages;
}


// *********************************************************************
// *                       Page: result: stereo                        *
// *********************************************************************
CalibrationWizardPageStereoResult::CalibrationWizardPageStereoResult (QWidget *parent)
    : QWizardPage(parent), fieldPrefix("Stereo")
{
    setTitle("Stereo calibration");
    setSubTitle("Calibration result");

    QLabel *label;

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Congratulations! Stereo parameters have been calibrated. One of calibration image pairs has been rectified and is shown below for visual validation of calibration result.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label);

    // Undistorted image
    displayImage = new ImagePairDisplayWidget("Rectified image pair", this);
    displayImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayImage);
}

CalibrationWizardPageStereoResult::~CalibrationWizardPageStereoResult ()
{
}

void CalibrationWizardPageStereoResult::initializePage ()
{
    // Get stereo calibration
    cv::Mat M1 = field(fieldPrefix + "CameraMatrix1").value<cv::Mat>();
    cv::Mat D1 = field(fieldPrefix + "DistCoeffs1").value<cv::Mat>();
    cv::Mat M2 = field(fieldPrefix + "CameraMatrix2").value<cv::Mat>();
    cv::Mat D2 = field(fieldPrefix + "DistCoeffs2").value<cv::Mat>();
    cv::Mat T = field(fieldPrefix + "T").value<cv::Mat>();
    cv::Mat R = field(fieldPrefix + "R").value<cv::Mat>();
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();

    // Initialize stereo rectification
    cv::Mat R1, R2;
    cv::Mat P1, P2;
    cv::Mat Q;
    cv::Rect validRoi1, validRoi2;
    cv::Mat map11, map12, map21, map22;

    cv::stereoRectify(M1, D1, M2, D2, imageSize, R, T, R1, R2, P1, P2, Q, cv::CALIB_ZERO_DISPARITY, 1, imageSize, &validRoi1, &validRoi2);

    cv::initUndistortRectifyMap(M1, D1, R1, P1, imageSize, CV_16SC2, map11, map12);
    cv::initUndistortRectifyMap(M2, D2, R2, P2, imageSize, CV_16SC2, map21, map22);
    
    // Load and rectify a pair
    QStringList images = field(fieldPrefix + "Images").toStringList();
    
    cv::Mat image1 = cv::imread(images[0].toStdString(), -1);
    cv::Mat image2 = cv::imread(images[1].toStdString(), -1);
    
    // Two simple remaps using look-up tables
    cv::Mat rectifiedImage1, rectifiedImage2;
    
    cv::remap(image1, rectifiedImage1, map11, map12, cv::INTER_LINEAR);
    cv::remap(image2, rectifiedImage2, map21, map22, cv::INTER_LINEAR);

    // Display undistorted image
    displayImage->setImagePair(rectifiedImage1, rectifiedImage2);
    displayImage->setImagePairROI(validRoi1, validRoi2);
}
