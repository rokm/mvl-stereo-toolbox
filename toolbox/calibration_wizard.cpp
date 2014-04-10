/*
 * MVL Stereo Toolbox: calibration wizard
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

#include "calibration_wizard.h"
#include "image_display_widget.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>


// *********************************************************************
// *                               Wizard                              *
// *********************************************************************
CalibrationWizard::CalibrationWizard (QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle("Calibration Wizard");
    setWizardStyle(ClassicStyle);

    resize(800, 600);

    setOption(QWizard::NoBackButtonOnStartPage, true);

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
    connect(pushButtonAddImages, SIGNAL(clicked()), this, SLOT(addImages()));

    imagesLayout->addWidget(pushButtonAddImages, 0, 0, 1, 1);

    // Clear images button
    pushButtonClearImages = new QPushButton("Clear");
    pushButtonClearImages->setToolTip("Clear images");
    connect(pushButtonClearImages, SIGNAL(clicked()), this, SLOT(clearImages()));

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
    comboBoxPatternType->addItem("Chessboard", StereoCalibrationPattern::Chessboard);
    comboBoxPatternType->addItem("Circle grid", StereoCalibrationPattern::Circles);
    comboBoxPatternType->addItem("Asymmetric circle grid", StereoCalibrationPattern::AsymmetricCircles);
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

    connect(this, SIGNAL(imagesChanged()), this, SIGNAL(completeChanged()));
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

    // Append images
    appendImages(fileNames);
}

void CalibrationWizardPageImages::clearImages ()
{
    listWidgetImages->clear();
    emit imagesChanged();
}

QStringList CalibrationWizardPageImages::getImages () const
{
    int numItems = listWidgetImages->count();
    QStringList images;

    for (int i = 0; i < numItems; i++) {
        images.append(listWidgetImages->item(i)->text());
    }

    return images;
}

void CalibrationWizardPageImages::setImages (const QStringList &fileNames)
{
    listWidgetImages->clear();

    appendImages(fileNames);
}

void CalibrationWizardPageImages::appendImages (const QStringList &fileNames)
{
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
    emit imagesChanged();
}

int CalibrationWizardPageImages::getPatternType () const
{
    return comboBoxPatternType->itemData(comboBoxPatternType->currentIndex()).toInt();
}

void CalibrationWizardPageImages::setPatternType (int type)
{
    comboBoxPatternType->setCurrentIndex(comboBoxPatternType->findData(type));
}


// *********************************************************************
// *                   Page: images selection: stereo                  *
// *********************************************************************
CalibrationWizardPageStereoImages::CalibrationWizardPageStereoImages (QWidget *parent)
    : CalibrationWizardPageImages("Stereo", parent)
{
    setTitle("Stereo calibration");

    // *** Images order ***
    QGroupBox *groupBox = new QGroupBox(tr("Images order"), this);
    QHBoxLayout *groupBoxLayout = new QHBoxLayout(groupBox);

    // Interleaved
    radioButtonInterleaved = new QRadioButton(tr("Interleaved"));
    radioButtonInterleaved->setToolTip(tr("Interleaved (L R L R L R ...)"));
    radioButtonInterleaved->setChecked(true); // Default
    connect(radioButtonInterleaved, &QRadioButton::toggled, this, [this] (bool checked) {
        if (checked) {
            setImagesOrder(Interleaved);
        }
    });
    groupBoxLayout->addWidget(radioButtonInterleaved);

    // Grouped
    radioButtonGrouped = new QRadioButton(tr("Grouped"));
    radioButtonGrouped->setToolTip(tr("Grouped (L L L ... R R R ...)"));
    connect(radioButtonGrouped, &QRadioButton::toggled, this, [this] (bool checked) {
        if (checked) {
            setImagesOrder(Grouped);
        }
    });
    groupBoxLayout->addWidget(radioButtonGrouped);

    // Stretch
    groupBoxLayout->addStretch(1);

    // Add to layout, store pointer, etc.
    //layout()->addWidget(groupBox, 2, 0, 1, 2);
    layout()->addWidget(groupBox);

    connect(this, SIGNAL(imagesChanged()), this, SLOT(updateImageEntries()));

    // Keep track of this setting via fields mechanism
    registerField(fieldPrefix + "ImagesOrder", this, "imagesOrder");
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


void CalibrationWizardPageStereoImages::initializePage ()
{
    CalibrationWizardPageImages::initializePage();

    if (!field("JointCalibration").toBool()) {
        // If in not in joint calibration, copy pattern settings from right camera
        QString rightCameraPrefix = "RightCamera";

        QStringList patternParameters;
        patternParameters << "PatternWidth" << "PatternHeight" << "ElementSize" << "PatternType" << "ScaleLevels" << "ScaleIncrement";

        foreach (QString fieldName, patternParameters) {
            setField(fieldPrefix + fieldName, field(rightCameraPrefix + fieldName));
        }
    }
}


QStringList CalibrationWizardPageStereoImages::getImages () const
{
    QStringList images;

    if (radioButtonInterleaved->isChecked()) {
        // Interleaved; we can use parent's function
        images = CalibrationWizardPageImages::getImages();
    } else {
        // Grouped; interleave them ourselves
        int numPairs = listWidgetImages->count() / 2;

        for (int i = 0; i < numPairs; i++) {
            images.append(listWidgetImages->item(i)->text());
            images.append(listWidgetImages->item(numPairs + i)->text());
        }
    }

    return images;
}


void CalibrationWizardPageStereoImages::setImagesOrder (int order)
{
    if (order == Interleaved) {
        radioButtonInterleaved->setChecked(true);
    } else {
        radioButtonGrouped->setChecked(true);
    }

    updateImageEntries();
}

int CalibrationWizardPageStereoImages::getImagesOrder () const
{
    if (radioButtonInterleaved->isChecked()) {
        return Interleaved;
    } else {
        return Grouped;
    }
}

void CalibrationWizardPageStereoImages::updateImageEntries ()
{
    int numItems = listWidgetImages->count();

    for (int i = 0; i < numItems; i++) {
        QListWidgetItem *item = listWidgetImages->item(i);

        if (radioButtonInterleaved->isChecked()) {
            // Interleaved
            if (i % 2) {
                item->setBackground(palette().alternateBase());
            } else {
                item->setBackground(palette().base());
            }
        } else {
            // Grouped
            if (i < numItems/2) {
                item->setBackground(palette().base());
            } else {
                item->setBackground(palette().alternateBase());
            }
        }
    }
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


void CalibrationWizardPageRightCameraImages::initializePage ()
{
    CalibrationWizardPageImages::initializePage();

    // Copy pattern parameters from left camera
    QString leftCameraPrefix = "LeftCamera";

    QStringList patternParameters;
    patternParameters << "PatternWidth" << "PatternHeight" << "ElementSize" << "PatternType" << "ScaleLevels" << "ScaleIncrement";

    foreach (QString fieldName, patternParameters) {
        setField(fieldPrefix + fieldName, field(leftCameraPrefix + fieldName));
    }
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
    QHBoxLayout *buttonBox = new QHBoxLayout();
    layout->addLayout(buttonBox, 3, 1, 1, 1);

    buttonBox->addStretch();

    pushButtonAuto = new QPushButton("Auto", this);
    pushButtonAuto->setCheckable(true);
    connect(pushButtonAuto, SIGNAL(toggled(bool)), this, SLOT(autoPatternToggled(bool)));
    buttonBox->addWidget(pushButtonAuto);

    pushButtonDiscard = new QPushButton("Discard", this);
    connect(pushButtonDiscard, SIGNAL(clicked()), this, SLOT(discardPatternClicked()));
    buttonBox->addWidget(pushButtonDiscard);

    pushButtonAccept = new QPushButton("Accept", this);
    connect(pushButtonAccept, SIGNAL(clicked()), this, SLOT(acceptPatternClicked()));
    buttonBox->addWidget(pushButtonAccept);

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

    patternFound = false;
    pushButtonAuto->setChecked(false);

    // ... and disable "Discard" button
    pushButtonDiscard->setEnabled(false);

    // Enable "auto" button
    pushButtonAuto->setEnabled(true);

    // Set parameters for pattern detector
    calibrationPattern.setParameters(
        field(fieldPrefix + "PatternWidth").toInt(),
        field(fieldPrefix + "PatternHeight").toInt(),
        field(fieldPrefix + "ElementSize").toDouble(),
        (StereoCalibrationPattern::PatternType)field(fieldPrefix + "PatternType").toInt(),
        field(fieldPrefix + "ScaleLevels").toInt(),
        field(fieldPrefix + "ScaleIncrement").toDouble()
    );
}


bool CalibrationWizardPageDetection::isComplete () const
{
    // We need at least six valid images/pairs (= world coordinate vectors)
    return patternWorldPoints.size() >= 6;
}


void CalibrationWizardPageDetection::startProcessing ()
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

void CalibrationWizardPageDetection::doAutomaticProcessing ()
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

void CalibrationWizardPageDetection::acceptPattern ()
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
            patternWorldPoints.push_back(calibrationPattern.computePlanarCoordinates());
        }
    } else {
        // Append image coordinates
        patternImagePoints.push_back(currentImagePoints);
        // Append world coordinates
        patternWorldPoints.push_back(calibrationPattern.computePlanarCoordinates());
    }

    // Process next
    imageCounter++;
    processImage();
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
    processImage();
}

void CalibrationWizardPageDetection::processImage ()
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
            return;
        }

        // Find pattern
        std::vector<cv::Point2f> points;
        patternFound = calibrationPattern.findInImage(image, currentImagePoints);

        if (!patternFound) {
            pushButtonAccept->hide();
            pushButtonAccept->setEnabled(false);
        }

        displayImage->setPattern(patternFound, currentImagePoints, calibrationPattern.getPatternSize());
    }

    // We might be ready to go on...
    emit completeChanged();

    // Auto process
    if (autoProcess) {
        QTimer::singleShot(500, this, SLOT(doAutomaticProcessing()));
    }
}


void CalibrationWizardPageDetection::autoPatternToggled (bool enable)
{
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
}

void CalibrationWizardPageDetection::acceptPatternClicked ()
{
    // Disable auto processing
    pushButtonAuto->setChecked(false);

    // Manual accept
    acceptPattern();
}

void CalibrationWizardPageDetection::discardPatternClicked ()
{
    // Disable auto processing
    pushButtonAuto->setChecked(false);

    // Manual discard
    discardPattern();
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
// *                     Calibration flags widget                      *
// *********************************************************************
CalibrationFlagsWidget::CalibrationFlagsWidget (const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    QFormLayout *groupBoxLayout = new QFormLayout(this);
    QCheckBox *checkBox;

    // CALIB_USE_INTRINSIC_GUESS
    checkBox = new QCheckBox("CALIB_USE_INTRINSIC_GUESS", this);
    checkBox->setToolTip("The input values for fx, fy, cx and cy are considered an initial guess \n"
                         "and are optimized further. Otherwise, (cx, cy) is initially set to the \n"
                         "image center and focal distances are computed in a least-squares fashion.");
    checkBoxUseIntrinsicGuess = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_PRINCIPAL_POINT
    checkBox = new QCheckBox("CALIB_FIX_PRINCIPAL_POINT", this);
    checkBox->setToolTip("The principal point is not changed during the global optimization.\n"
                         "It stays at the center or, if CALIB_USE_INTRINSIC_GUESS is set, at \n"
                         "the specified location.");
    checkBoxFixPrincipalPoint = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_ASPECT_RATIO
    checkBox = new QCheckBox("CALIB_FIX_ASPECT_RATIO", this);
    checkBox->setToolTip("The function considers only fy as a free parameter. The ratio fx/fy\n"
                         "stays the same as defined by the input values. If CALIB_USE_INTRINSIC_GUESS \n"
                         "is not set, the actual input values of fx and fy are ignored, and only their \n"
                         "ratio is computed and used further.");
    checkBoxFixAspectRatio = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_ZERO_TANGENT_DIST
    checkBox = new QCheckBox("CALIB_ZERO_TANGENT_DIST", this);
    checkBox->setToolTip("Tangential distortion coefficients (p1, p2) are set to zeros and stay zero.");
    checkBoxZeroTangentDist = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_RATIONAL_MODEL
    checkBox = new QCheckBox("CALIB_RATIONAL_MODEL", this);
    checkBox->setToolTip("Coefficients k4, k5, and k6 are enabled, totally amounting to 8 distortion coefficients.\n"
                         "If the flag is not set, the function computes and returns only 5 distortion coefficients.");
    checkBoxRationalModel = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K1
    checkBox = new QCheckBox("CALIB_FIX_K1", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK1 = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K2
    checkBox = new QCheckBox("CALIB_FIX_K2", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK2 = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K3
    checkBox = new QCheckBox("CALIB_FIX_K3", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK3 = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K4
    checkBox = new QCheckBox("CALIB_FIX_K4", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK4 = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K5
    checkBox = new QCheckBox("CALIB_FIX_K5", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK5 = checkBox;
    groupBoxLayout->addRow(checkBox);

    // CALIB_FIX_K6
    checkBox = new QCheckBox("CALIB_FIX_K6", this);
    checkBox->setToolTip("The corresponding radial distortion coefficient is not changed during the optimization.\n"
                         "If CALIB_USE_INTRINSIC_GUESS is set, the supplied coefficient value is used. Otherwise, \n"
                         "it is set to zero.");
    checkBoxFixK6 = checkBox;
    groupBoxLayout->addRow(checkBox);
}

CalibrationFlagsWidget::~CalibrationFlagsWidget ()
{
}


int CalibrationFlagsWidget::getFlags () const
{
    int flags = 0;

    flags |= (checkBoxUseIntrinsicGuess->checkState() == Qt::Checked) * cv::CALIB_USE_INTRINSIC_GUESS;
    flags |= (checkBoxFixPrincipalPoint->checkState() == Qt::Checked) * cv::CALIB_FIX_PRINCIPAL_POINT;
    flags |= (checkBoxFixAspectRatio->checkState() == Qt::Checked) * cv::CALIB_FIX_ASPECT_RATIO;
    flags |= (checkBoxZeroTangentDist->checkState() == Qt::Checked) * cv::CALIB_ZERO_TANGENT_DIST;
    flags |= (checkBoxRationalModel->checkState() == Qt::Checked) * cv::CALIB_RATIONAL_MODEL;
    flags |= (checkBoxFixK1->checkState() == Qt::Checked) * cv::CALIB_FIX_K1;
    flags |= (checkBoxFixK2->checkState() == Qt::Checked) * cv::CALIB_FIX_K2;
    flags |= (checkBoxFixK3->checkState() == Qt::Checked) * cv::CALIB_FIX_K3;
    flags |= (checkBoxFixK4->checkState() == Qt::Checked) * cv::CALIB_FIX_K4;
    flags |= (checkBoxFixK5->checkState() == Qt::Checked) * cv::CALIB_FIX_K5;
    flags |= (checkBoxFixK6->checkState() == Qt::Checked) * cv::CALIB_FIX_K6;

    return flags;
}

void CalibrationFlagsWidget::setFlags (int flags)
{
    checkBoxUseIntrinsicGuess->setChecked(flags & cv::CALIB_USE_INTRINSIC_GUESS);
    checkBoxFixPrincipalPoint->setChecked(flags & cv::CALIB_FIX_PRINCIPAL_POINT);
    checkBoxFixAspectRatio->setChecked(flags & cv::CALIB_FIX_ASPECT_RATIO);
    checkBoxZeroTangentDist->setChecked(flags & cv::CALIB_ZERO_TANGENT_DIST);
    checkBoxRationalModel->setChecked(flags & cv::CALIB_RATIONAL_MODEL);
    checkBoxFixK1->setChecked(flags & cv::CALIB_FIX_K1);
    checkBoxFixK2->setChecked(flags & cv::CALIB_FIX_K2);
    checkBoxFixK3->setChecked(flags & cv::CALIB_FIX_K3);
    checkBoxFixK4->setChecked(flags & cv::CALIB_FIX_K4);
    checkBoxFixK5->setChecked(flags & cv::CALIB_FIX_K5);
    checkBoxFixK6->setChecked(flags & cv::CALIB_FIX_K6);
}


// *********************************************************************
// *                  Stereo calibration flags widget                  *
// *********************************************************************
StereoCalibrationFlagsWidget::StereoCalibrationFlagsWidget (const QString &title, QWidget *parent)
    : CalibrationFlagsWidget(title, parent)
{
    QFormLayout *groupBoxLayout = qobject_cast<QFormLayout *>(layout());
    QCheckBox *checkBox;

    // CALIB_FIX_INTRINSIC
    checkBox = new QCheckBox("CALIB_FIX_INTRINSIC", this);
    checkBox->setToolTip("Fix camera matrices and distortion coefficients, so that only inter-camera rotation and \n"
                         "translation matrices (R and T) are estimated.");
    checkBoxFixIntrinsic = checkBox;
    groupBoxLayout->insertRow(0, checkBox);

    // CALIB_FIX_FOCAL_LENGTH
    checkBox = new QCheckBox("CALIB_FIX_FOCAL_LENGTH", this);
    checkBox->setToolTip("Fix values of fx and fy on both cameras.");
    checkBoxFixFocalLength = checkBox;
    groupBoxLayout->insertRow(3, checkBox);

    // CALIB_SAME_FOCAL_LENGTH
    checkBox = new QCheckBox("CALIB_SAME_FOCAL_LENGTH", this);
    checkBox->setToolTip("Enforce same focal length on both cameras.");
    checkBoxSameFocalLength = checkBox;
    groupBoxLayout->insertRow(6, checkBox);
}

StereoCalibrationFlagsWidget::~StereoCalibrationFlagsWidget ()
{
}


int StereoCalibrationFlagsWidget::getFlags () const
{
    int flags = CalibrationFlagsWidget::getFlags();

    flags |= (checkBoxFixIntrinsic->checkState() == Qt::Checked) * cv::CALIB_FIX_INTRINSIC;
    flags |= (checkBoxFixFocalLength->checkState() == Qt::Checked) * cv::CALIB_FIX_FOCAL_LENGTH;
    flags |= (checkBoxSameFocalLength->checkState() == Qt::Checked) * cv::CALIB_SAME_FOCAL_LENGTH;

    return flags;
}

void StereoCalibrationFlagsWidget::setFlags (int flags)
{
    CalibrationFlagsWidget::setFlags(flags);

    checkBoxFixIntrinsic->setChecked(flags & cv::CALIB_FIX_INTRINSIC);
    checkBoxFixFocalLength->setChecked(flags & cv::CALIB_FIX_FOCAL_LENGTH);
    checkBoxSameFocalLength->setChecked(flags & cv::CALIB_SAME_FOCAL_LENGTH);
}



// *********************************************************************
// *                     Camera parameters widget                      *
// *********************************************************************
CameraParametersWidget::CameraParametersWidget (const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    QFormLayout *groupBoxLayout = new QFormLayout(this);
    QLabel *label;
    QDoubleSpinBox *spinBoxD;

    // Fx
    label = new QLabel("fx", this);
    label->setToolTip("Focal length (horizontal) in pixel units");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(2);
    spinBoxD->setSuffix(" px");
    spinBoxFx = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // Fy
    label = new QLabel("fy", this);
    label->setToolTip("Focal length (vertical) in pixel units");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(2);
    spinBoxD->setSuffix(" px");
    spinBoxFy = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // Cx
    label = new QLabel("cx", this);
    label->setToolTip("Horizontal coordinate of principal point");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(2);
    spinBoxD->setSuffix(" px");
    spinBoxCx = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // Cy
    label = new QLabel("cy", this);
    label->setToolTip("Vertical coordinate of principal point");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(2);
    spinBoxD->setSuffix(" px");
    spinBoxCy = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // K1
    label = new QLabel("k1", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK1 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // K2
    label = new QLabel("k2", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK2 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // P1
    label = new QLabel("p1", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxP1 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);


    // P2
    label = new QLabel("p2", this);
    label->setToolTip("Tangential distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxP2 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);


    // K3
    label = new QLabel("k3", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK3 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // K4
    label = new QLabel("k4", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK4 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // K5
    label = new QLabel("k5", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK5 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);

    // K6
    label = new QLabel("k6", this);
    label->setToolTip("Radial distortion coefficient");

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setRange(-9999.0, 9999.0);
    spinBoxD->setValue(0);
    spinBoxD->setDecimals(4);
    spinBoxK6 = spinBoxD;

    groupBoxLayout->addRow(label, spinBoxD);
}

CameraParametersWidget::~CameraParametersWidget ()
{
}

double CameraParametersWidget::getFocalLengthX () const
{
    return spinBoxFx->value();
}

void CameraParametersWidget::setFocalLengthX (double newValue)
{
    spinBoxFx->setValue(newValue);
}

double CameraParametersWidget::getFocalLengthY () const
{
    return spinBoxFy->value();
}

void CameraParametersWidget::setFocalLengthY (double newValue)
{
    spinBoxFy->setValue(newValue);
}


double CameraParametersWidget::getPrincipalPointX () const
{
    return spinBoxCx->value();
}

void CameraParametersWidget::setPrincipalPointX (double newValue)
{
    spinBoxCx->setValue(newValue);
}


double CameraParametersWidget::getPrincipalPointY () const
{
    return spinBoxCy->value();
}

void CameraParametersWidget::setPrincipalPointY (double newValue)
{
    spinBoxCy->setValue(newValue);
}


double CameraParametersWidget::getDistortionK1 () const
{
    return spinBoxK1->value();
}

void CameraParametersWidget::setDistortionK1 (double newValue)
{
    spinBoxK1->setValue(newValue);
}


double CameraParametersWidget::getDistortionK2 () const
{
    return spinBoxK2->value();
}

void CameraParametersWidget::setDistortionK2 (double newValue)
{
    spinBoxK2->setValue(newValue);
}


double CameraParametersWidget::getDistortionK3 () const
{
    return spinBoxK3->value();
}

void CameraParametersWidget::setDistortionK3 (double newValue)
{
    spinBoxK3->setValue(newValue);
}


double CameraParametersWidget::getDistortionP1 () const
{
    return spinBoxP1->value();
}

void CameraParametersWidget::setDistortionP1 (double newValue)
{
    spinBoxP1->setValue(newValue);
}


double CameraParametersWidget::getDistortionP2 () const
{
    return spinBoxP2->value();
}

void CameraParametersWidget::setDistortionP2 (double newValue)
{
    spinBoxP2->setValue(newValue);
}


double CameraParametersWidget::getDistortionK4 () const
{
    return spinBoxK4->value();
}

void CameraParametersWidget::setDistortionK4 (double newValue)
{
    spinBoxK4->setValue(newValue);
}


double CameraParametersWidget::getDistortionK5 () const
{
    return spinBoxK5->value();
}

void CameraParametersWidget::setDistortionK5 (double newValue)
{
    spinBoxK5->setValue(newValue);
}


double CameraParametersWidget::getDistortionK6 () const
{
    return spinBoxK6->value();
}

void CameraParametersWidget::setDistortionK6 (double newValue)
{
    spinBoxK6->setValue(newValue);
}

void CameraParametersWidget::setCameraMatrix (const cv::Mat &cameraMatrix, const std::vector<double> &distCoeffs)
{
    // Read values of interest from camera matrix
    setFocalLengthX(cameraMatrix.at<double>(0, 0));
    setFocalLengthY(cameraMatrix.at<double>(1, 1));
    setPrincipalPointX(cameraMatrix.at<double>(0, 2));
    setPrincipalPointY(cameraMatrix.at<double>(1, 2));

    // K1, K2, P1 and P2 are always present
    setDistortionK1(distCoeffs[0]);
    setDistortionK2(distCoeffs[1]);
    setDistortionP1(distCoeffs[2]);
    setDistortionP2(distCoeffs[3]);

    // K3 is optional
    if (distCoeffs.size() > 4) {
        setDistortionK3(distCoeffs[4]);
    } else {
        setDistortionK3(0);
    }

    // K4, K5 and K6 are present only for rational models
    if (distCoeffs.size() > 5) {
        setDistortionK4(distCoeffs[5]);
        setDistortionK5(distCoeffs[6]);
        setDistortionK6(distCoeffs[7]);
    } else {
        setDistortionK4(0);
        setDistortionK5(0);
        setDistortionK6(0);
    }
}

cv::Mat CameraParametersWidget::getCameraMatrix () const
{
    cv::Mat M = cv::Mat::eye(3, 3, CV_64F);

    M.at<double>(0, 0) = getFocalLengthX();
    M.at<double>(1, 1) = getFocalLengthX();
    M.at<double>(0, 2) = getPrincipalPointX();
    M.at<double>(1, 2) = getPrincipalPointY();

    return M;
}

std::vector<double> CameraParametersWidget::getDistCoeffs () const
{
    std::vector<double> D(8);

    D[0] = getDistortionK1();
    D[1] = getDistortionK2();
    D[2] = getDistortionP1();
    D[3] = getDistortionP2();
    D[4] = getDistortionK3();
    D[5] = getDistortionK4();
    D[6] = getDistortionK5();
    D[7] = getDistortionK6();

    return D;
}


void CameraParametersWidget::setDisplayMode (bool displayMode)
{
    if (displayMode) {
        spinBoxFx->setEnabled(false);
        spinBoxFy->setEnabled(false);
        spinBoxCx->setEnabled(false);
        spinBoxCy->setEnabled(false);
        spinBoxK1->setEnabled(false);
        spinBoxK2->setEnabled(false);
        spinBoxP1->setEnabled(false);
        spinBoxP2->setEnabled(false);
        spinBoxK3->setEnabled(false);
        spinBoxK4->setEnabled(false);
        spinBoxK5->setEnabled(false);
        spinBoxK6->setEnabled(false);
    } else {
        spinBoxFx->setEnabled(true);
        spinBoxFy->setEnabled(true);
        spinBoxCx->setEnabled(true);
        spinBoxCy->setEnabled(true);
        spinBoxK1->setEnabled(true);
        spinBoxK2->setEnabled(true);
        spinBoxP1->setEnabled(true);
        spinBoxP2->setEnabled(true);
        spinBoxK3->setEnabled(true);
        spinBoxK4->setEnabled(true);
        spinBoxK5->setEnabled(true);
        spinBoxK6->setEnabled(true);
    }
}


// *********************************************************************
// *       Page: calibration: common class for left/right camera       *
// *********************************************************************
CalibrationWizardPageCalibration::CalibrationWizardPageCalibration (const QString &fieldPrefixString, QWidget *parent)
    : QWizardPage(parent), fieldPrefix(fieldPrefixString)
{
    setSubTitle("Calibration parameters");

    // Busy dialog
    dialogBusy = new QProgressDialog("Calibrating... please wait", QString(), 0, 0, this);
    dialogBusy->setModal(true);

    // Worker thread
    calibrationComplete = false;
    connect(&calibrationWatcher, SIGNAL(finished()), this, SLOT(calibrationFinished()));

    QLabel *label;

    // Layout
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Please enter parameters for calibration method, and press \"Calibrate\" to perform calibration. Depending on "
                       "number of images and hardware, the operation might take some time.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label, 0, 0, 1, 2);

    // Camera parameters
    boxCameraParameters = new CameraParametersWidget("Parameters", this);
    layout->addWidget(boxCameraParameters, 1, 0, 1, 1);

    // Calibration Flags
    boxCalibrationFlags = new CalibrationFlagsWidget("Flags", this);
    layout->addWidget(boxCalibrationFlags, 1, 1, 1, 1);

    boxCalibrationFlags->setFlags(cv::CALIB_RATIONAL_MODEL); // Default flags

    // Fields
    registerField(fieldPrefix + "CameraMatrix", this, "cameraMatrix");
    registerField(fieldPrefix + "DistCoeffs", this, "distCoeffs");

    // Needed for transfer between pages
    registerField(fieldPrefix + "CalibrationFlags", this, "calibrationFlags");
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


void CalibrationWizardPageCalibration::setCalibrationFlags (int flags)
{
    boxCalibrationFlags->setFlags(flags);
}

int CalibrationWizardPageCalibration::getCalibrationFlags () const
{
    return boxCalibrationFlags->getFlags();
}


void CalibrationWizardPageCalibration::setVisible (bool visible)
{
    QWizardPage::setVisible(visible);

    // On Windows, this function gets called without wizard being set...
    if (!wizard()) {
        return;
    }

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("&Calibrate"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        connect(wizard(), SIGNAL(customButtonClicked(int)), this, SLOT(calibrationClicked()));
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        disconnect(wizard(), SIGNAL(customButtonClicked(int)), this, SLOT(calibrationClicked()));
    }
}

void CalibrationWizardPageCalibration::initializePage ()
{
    // Initialize principal point in the camera parameters widget
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();
    boxCameraParameters->setPrincipalPointX(imageSize.width/2);
    boxCameraParameters->setPrincipalPointY(imageSize.height/2);

    calibrationComplete = false;
}

bool CalibrationWizardPageCalibration::isComplete () const
{
    // Ready when calibration has been computed
    return calibrationComplete;
}


void CalibrationWizardPageCalibration::calibrationClicked ()
{
    // Show busy dialog
    dialogBusy->show();

    // Start calibration function
    QFuture<bool> future = QtConcurrent::run(this, &CalibrationWizardPageCalibration::calibrationFunction);
    calibrationWatcher.setFuture(future);
}

void CalibrationWizardPageCalibration::calibrationFinished ()
{
    dialogBusy->hide();

    // Read result
    calibrationComplete = calibrationWatcher.result();

    // Display dialog
    if (calibrationComplete) {
        QMessageBox::information(this, "Calibration finished", QString("Calibration finished!\nRMSE: %1").arg(calibrationRMSE));
    }

    // We might be ready
    emit completeChanged();
}

bool CalibrationWizardPageCalibration::calibrationFunction ()
{
    // Reset calibration
    calibrationComplete = false;

    std::vector<std::vector<cv::Point2f> > imagePoints = field(fieldPrefix + "PatternImagePoints").value< std::vector<std::vector<cv::Point2f> > >();
    std::vector<std::vector<cv::Point3f> > worldPoints = field(fieldPrefix + "PatternWorldPoints").value< std::vector<std::vector<cv::Point3f> > >();
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();

    // Get values from the config widgets
    int flags = boxCalibrationFlags->getFlags();
    cameraMatrix = boxCameraParameters->getCameraMatrix();
    distCoeffs = cv::Mat(boxCameraParameters->getDistCoeffs()).clone();

    try {
        calibrationRMSE = cv::calibrateCamera(worldPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, cv::noArray(), cv::noArray(), flags);
    } catch (cv::Exception e) {
        emit error("Calibration failed: " + QString::fromStdString(e.what()));
        return false;
    }

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

void CalibrationWizardPageRightCameraCalibration::initializePage ()
{
    CalibrationWizardPageCalibration::initializePage();

    // Copy calibration flags setting from left camera
    QString leftCameraPrefix = "LeftCamera";
    setField(fieldPrefix + "CalibrationFlags", field(leftCameraPrefix + "CalibrationFlags"));
}


// *********************************************************************
// *                     Page: calibration: stereo                     *
// *********************************************************************
CalibrationWizardPageStereoCalibration::CalibrationWizardPageStereoCalibration (QWidget *parent)
    : QWizardPage(parent), fieldPrefix("Stereo")
{
    setSubTitle("Calibration parameters");

    // Busy dialog
    dialogBusy = new QProgressDialog("Calibrating... please wait", QString(), 0, 0, this);
    dialogBusy->setModal(true);

    // Worker thread
    calibrationComplete = false;
    connect(&calibrationWatcher, SIGNAL(finished()), this, SLOT(calibrationFinished()));

    QLabel *label;

    // Layout
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(10);

    // Label
    label = new QLabel("Please enter parameters for calibration method, and press \"Calibrate\" to perform calibration. Depending on "
                       "number of images and hardware, the operation might take some time, during which the GUI will appear to be frozen.");
    label->setAlignment(Qt::AlignVCenter | Qt::AlignJustify);
    label->setWordWrap(true);

    layout->addWidget(label, 0, 0, 1, 3);

    // Left camera parameters
    boxLeftCameraParameters = new CameraParametersWidget("Left camera", this);
    layout->addWidget(boxLeftCameraParameters, 1, 0, 1, 1);

    // Right camera parameters
    boxRightCameraParameters = new CameraParametersWidget("Right camera", this);
    layout->addWidget(boxRightCameraParameters, 1, 1, 1, 1);

    // Calibration flags
    boxCalibrationFlags = new StereoCalibrationFlagsWidget("Flags", this);
    layout->addWidget(boxCalibrationFlags, 1, 2, 1, 1);

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


void CalibrationWizardPageStereoCalibration::setVisible (bool visible)
{
    QWizardPage::setVisible(visible);

    // On Windows, this function gets called without wizard being set...
    if (!wizard()) {
        return;
    }

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("&Calibrate"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        connect(wizard(), SIGNAL(customButtonClicked(int)), this, SLOT(calibrationClicked()));
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        disconnect(wizard(), SIGNAL(customButtonClicked(int)), this, SLOT(calibrationClicked()));
    }
}


void CalibrationWizardPageStereoCalibration::initializePage ()
{
    if (field("JointCalibration").toBool()) {
        // Initialize principal point in the camera parameters widget
        cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();
        boxLeftCameraParameters->setPrincipalPointX(imageSize.width/2);
        boxLeftCameraParameters->setPrincipalPointY(imageSize.height/2);

        boxRightCameraParameters->setPrincipalPointX(imageSize.width/2);
        boxRightCameraParameters->setPrincipalPointY(imageSize.height/2);

        boxCalibrationFlags->setFlags(cv::CALIB_RATIONAL_MODEL); // Default flags
    } else {
        // Disjoint calibration; load camera matrices and distortion
        // coefficients, and set CALIB_FIX_INTRINSIC by default
        cv::Mat M1 = field("LeftCameraCameraMatrix").value<cv::Mat>();
        cv::Mat D1 = field("LeftCameraDistCoeffs").value<cv::Mat>();
        boxLeftCameraParameters->setCameraMatrix(M1, D1);

        cv::Mat M2 = field("RightCameraCameraMatrix").value<cv::Mat>();
        cv::Mat D2 = field("RightCameraDistCoeffs").value<cv::Mat>();
        boxRightCameraParameters->setCameraMatrix(M2, D2);

        boxCalibrationFlags->setFlags(cv::CALIB_RATIONAL_MODEL | cv::CALIB_FIX_INTRINSIC); // Default flags
    }

    calibrationComplete = false;
}

bool CalibrationWizardPageStereoCalibration::isComplete () const
{
    // Ready when calibration has been computed
    return calibrationComplete;
}


void CalibrationWizardPageStereoCalibration::calibrationClicked ()
{
    // Show busy dialog
    dialogBusy->show();

    // Start calibration function
    QFuture<bool> future = QtConcurrent::run(this, &CalibrationWizardPageStereoCalibration::calibrationFunction);
    calibrationWatcher.setFuture(future);
}

void CalibrationWizardPageStereoCalibration::calibrationFinished ()
{
    dialogBusy->hide();

    // Read result
    calibrationComplete = calibrationWatcher.result();

    // Display dialog
    if (calibrationComplete) {
        QMessageBox::information(this, "Calibration finished", QString("Calibration finished!\nRMSE: %1\nAvg. reprojection error: %2").arg(calibrationRMSE).arg(averageReprojectionError));
    }

    // We might be ready
    emit completeChanged();
}


bool CalibrationWizardPageStereoCalibration::calibrationFunction ()
{
    // Reset calibration
    calibrationComplete = false;

    std::vector<std::vector<cv::Point2f> > imagePoints = field(fieldPrefix + "PatternImagePoints").value< std::vector<std::vector<cv::Point2f> > >();
    std::vector<std::vector<cv::Point3f> > objectPoints = field(fieldPrefix + "PatternWorldPoints").value< std::vector<std::vector<cv::Point3f> > >();
    cv::Size imageSize = field(fieldPrefix + "ImageSize").value<cv::Size>();

    // Perform calibration here - we use OpenCV directly and bypass
    // the pipeline's stereo calibration function, so that calibration
    // can be cancelled at any point without affecting the pipeline...

    // We need to split image points in two
    std::vector<std::vector<cv::Point2f> > imagePoints1;
    std::vector<std::vector<cv::Point2f> > imagePoints2;

    for (unsigned int i = 0; i < objectPoints.size(); i++) {
        imagePoints1.push_back(imagePoints[2*i]);
        imagePoints2.push_back(imagePoints[2*i+1]);
    }

    // Get values from the config widgets
    int flags = boxCalibrationFlags->getFlags();
    cameraMatrix1 = boxLeftCameraParameters->getCameraMatrix();
    distCoeffs1 = cv::Mat(boxLeftCameraParameters->getDistCoeffs()).clone();
    cameraMatrix2 = boxRightCameraParameters->getCameraMatrix();
    distCoeffs2 = cv::Mat(boxRightCameraParameters->getDistCoeffs()).clone();

    cv::Mat E, F;

    try {
        calibrationRMSE = cv::stereoCalibrate(objectPoints, imagePoints1, imagePoints2,
                                              cameraMatrix1, distCoeffs1,
                                              cameraMatrix2, distCoeffs2,
                                              imageSize, R, T, E, F,
                                              cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-6),
                                              flags);
    } catch (cv::Exception e) {
        emit error("Calibration failed: " + QString::fromStdString(e.what()));
        return false;
    }

    // Compute average reprojection error
    averageReprojectionError = 0;
    int num_points = 0;
    std::vector<cv::Vec3f> lines1, lines2;

    for (unsigned int i = 0; i < objectPoints.size(); i++) {
        int npt = imagePoints1[i].size();
        cv::Mat imgpt1, imgpt2;

        // First image
        imgpt1 = cv::Mat(imagePoints1[i]);
        cv::undistortPoints(imgpt1, imgpt1, cameraMatrix1, distCoeffs1, cv::Mat(), cameraMatrix1);
        cv::computeCorrespondEpilines(imgpt1, 1, F, lines1);

        // Second image
        imgpt2 = cv::Mat(imagePoints2[i]);
        cv::undistortPoints(imgpt2, imgpt2, cameraMatrix2, distCoeffs2, cv::Mat(), cameraMatrix2);
        cv::computeCorrespondEpilines(imgpt2, 2, F, lines2);

        // Compute error
        for (int j = 0; j < npt; j++) {
            double errij = fabs(imagePoints1[i][j].x*lines2[j][0] +
                                imagePoints1[i][j].y*lines2[j][1] + lines2[j][2]) +
                           fabs(imagePoints2[i][j].x*lines1[j][0] +
                                imagePoints2[i][j].y*lines1[j][1] + lines1[j][2]);
            averageReprojectionError += errij;
        }

        num_points += npt;
    }
    averageReprojectionError /= num_points;

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

    // Splitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(splitter);

    // Camera parameters
    boxCameraParameters = new CameraParametersWidget("Parameters", this);
    boxCameraParameters->setDisplayMode(true);
    splitter->addWidget(boxCameraParameters);

    // Undistorted image
    displayImage = new ImageDisplayWidget("Undistorted image", this);
    displayImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    displayImage->resize(400, 600);
    splitter->addWidget(displayImage);
}

CalibrationWizardPageResult::~CalibrationWizardPageResult ()
{
}

void CalibrationWizardPageResult::initializePage ()
{
    // Get camera calibration
    cv::Mat M = field(fieldPrefix + "CameraMatrix").value<cv::Mat>();
    cv::Mat D = field(fieldPrefix + "DistCoeffs").value<cv::Mat>();

    // Display parameters
    boxCameraParameters->setCameraMatrix(M, D);

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

    // Splitter
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(splitter);

    // Undistorted image
    displayImage = new ImagePairDisplayWidget("Rectified image pair", this);
    displayImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    displayImage->resize(400, 600);
    splitter->addWidget(displayImage);

    // Scroll area; camera parameters
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);

    QWidget *parametersWidget = new QWidget(scrollArea);
    QHBoxLayout *boxLayout = new QHBoxLayout(parametersWidget);
    parametersWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    boxLayout->setContentsMargins(0, 0, 0, 0);

    // Left camera parameters
    boxLeftCameraParameters = new CameraParametersWidget("Left camera", this);
    boxLeftCameraParameters->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    boxLeftCameraParameters->setDisplayMode(true);
    boxLayout->addWidget(boxLeftCameraParameters);

    //boxLayout->addStretch();

    // Right camera parameters
    boxRightCameraParameters = new CameraParametersWidget("Right camera", this);
    boxRightCameraParameters->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    boxRightCameraParameters->setDisplayMode(true);
    boxLayout->addWidget(boxRightCameraParameters);

    scrollArea->setWidget(parametersWidget);
    splitter->addWidget(scrollArea);
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

    // Display parameters
    boxLeftCameraParameters->setCameraMatrix(M1, D1);
    boxRightCameraParameters->setCameraMatrix(M2, D2);

    // Initialize stereo rectification
    cv::Mat R1, R2;
    cv::Mat P1, P2;
    cv::Mat Q;
    cv::Rect validRoi1, validRoi2;
    cv::Mat map11, map12, map21, map22;

    cv::stereoRectify(M1, D1, M2, D2, imageSize, R, T, R1, R2, P1, P2, Q, cv::CALIB_ZERO_DISPARITY, 0, imageSize, &validRoi1, &validRoi2);

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
