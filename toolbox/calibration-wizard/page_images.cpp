/*
 * MVL Stereo Toolbox: calibration wizard: images selection page
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


#include "page_images.h"
#include "wizard.h"

#include <stereo-pipeline/calibration_pattern.h>


namespace MVL {
namespace StereoToolbox {
namespace GUI {
namespace CalibrationWizard {


// *********************************************************************
// *                    Image selection page: common                   *
// *********************************************************************
PageImages::PageImages (const QString &fieldPrefixString, QWidget *parent)
    : QWizardPage(parent),
      fieldPrefix(fieldPrefixString)
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
    connect(pushButtonAddImages, &QPushButton::clicked, this, &PageImages::addImages);

    imagesLayout->addWidget(pushButtonAddImages, 0, 0, 1, 1);

    // Clear images button
    pushButtonClearImages = new QPushButton("Clear");
    pushButtonClearImages->setToolTip("Clear images");
    connect(pushButtonClearImages, &QPushButton::clicked, this, &PageImages::clearImages);

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
    comboBoxPatternType->addItem("Chessboard", Pipeline::CalibrationPattern::Chessboard);
    comboBoxPatternType->addItem("Circle grid", Pipeline::CalibrationPattern::Circles);
    comboBoxPatternType->addItem("Asymmetric circle grid", Pipeline::CalibrationPattern::AsymmetricCircles);
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

    connect(this, &PageImages::imagesChanged, this, &PageImages::completeChanged);
}

PageImages::~PageImages ()
{
}

void PageImages::addImages ()
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

            QDir listFileDir = QFileInfo(fileNames[0]).absoluteDir();

            // Clear file names
            fileNames.clear();

            while (1) {
                line = stream.readLine();
                if (line.isNull()) {
                    break;
                } else {
                    if (QFileInfo(line).isRelative()) {
                        fileNames << listFileDir.absoluteFilePath(line);
                    } else {
                        fileNames << line;
                    }
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

void PageImages::clearImages ()
{
    listWidgetImages->clear();
    emit imagesChanged();
}

QStringList PageImages::getImages () const
{
    int numItems = listWidgetImages->count();
    QStringList images;

    images.reserve(numItems);
    for (int i = 0; i < numItems; i++) {
        images.append(listWidgetImages->item(i)->text());
    }

    return images;
}

void PageImages::setImages (const QStringList &filenames)
{
    listWidgetImages->clear();

    appendImages(filenames);
}

void PageImages::appendImages (const QStringList &filenames)
{
    // Display them in the list
    for (const QString &filename : filenames) {
        QFileInfo info(filename);

        QListWidgetItem *item = new QListWidgetItem(filename, listWidgetImages);
        listWidgetImages->addItem(item);

        if (!info.isReadable()) {
            item->setForeground(Qt::red);
            item->setToolTip("File cannot be read!");
        }
    }

    // Revalidate
    emit imagesChanged();
}

int PageImages::getPatternType () const
{
    return comboBoxPatternType->itemData(comboBoxPatternType->currentIndex()).toInt();
}

void PageImages::setPatternType (int type)
{
    comboBoxPatternType->setCurrentIndex(comboBoxPatternType->findData(type));
}


// *********************************************************************
// *                 Image selection page: single camera               *
// *********************************************************************
PageSingleCameraImages::PageSingleCameraImages (QWidget *parent)
    : PageImages("SingleCamera", parent)
{
    setTitle("Single camera calibration");
}

PageSingleCameraImages::~PageSingleCameraImages ()
{
}

int PageSingleCameraImages::nextId () const
{
    return Wizard::PageId::SingleCameraDetectionId;
}

bool PageSingleCameraImages::isComplete () const
{
    QStringList filenames = getImages();

    // Must be at least six images
    if (filenames.size() < 6) {
        return false;
    }

    return true;
}


// *********************************************************************
// *                  Image selection page: left camera                *
// *********************************************************************
PageLeftCameraImages::PageLeftCameraImages (QWidget *parent)
    : PageImages("LeftCamera", parent)
{
    setTitle("Left camera calibration");
}

PageLeftCameraImages::~PageLeftCameraImages ()
{
}

int PageLeftCameraImages::nextId () const
{
    return Wizard::PageId::LeftCameraDetectionId;
}

bool PageLeftCameraImages::isComplete () const
{
    QStringList filenames = getImages();

    // Must be at least six images
    if (filenames.size() < 6) {
        return false;
    }

    return true;
}


// *********************************************************************
// *                 Image selection page: right camera                *
// *********************************************************************
PageRightCameraImages::PageRightCameraImages (QWidget *parent)
    : PageImages("RightCamera", parent)
{
    setTitle("Right camera calibration");
}

PageRightCameraImages::~PageRightCameraImages ()
{
}

int PageRightCameraImages::nextId () const
{
    return Wizard::PageId::RightCameraDetectionId;
}

bool PageRightCameraImages::isComplete () const
{
    QStringList filenames = getImages();

    // Must be at least six images
    if (filenames.size() < 6) {
        return false;
    }

    return true;
}


void PageRightCameraImages::initializePage ()
{
    PageImages::initializePage();

    // Copy pattern parameters from left camera
    QString leftCameraPrefix = "LeftCamera";

    QStringList patternParameters;
    patternParameters << "PatternWidth" << "PatternHeight" << "ElementSize" << "PatternType" << "ScaleLevels" << "ScaleIncrement";

    for (const QString &fieldName : patternParameters) {
        setField(fieldPrefix + fieldName, field(leftCameraPrefix + fieldName));
    }
}


// *********************************************************************
// *                    Image selection page: stereo                   *
// *********************************************************************
PageStereoImages::PageStereoImages (QWidget *parent)
    : PageImages("Stereo", parent)
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

    connect(this, &PageStereoImages::imagesChanged, this, &PageStereoImages::updateImageEntries);

    // Keep track of this setting via fields mechanism
    registerField(fieldPrefix + "ImagesOrder", this, "imagesOrder");
}

PageStereoImages::~PageStereoImages ()
{
}

int PageStereoImages::nextId () const
{
    return Wizard::PageId::StereoDetectionId;
}

bool PageStereoImages::isComplete () const
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


void PageStereoImages::initializePage ()
{
    PageImages::initializePage();

    if (!field("JointCalibration").toBool()) {
        // If in not in joint calibration, copy pattern settings from right camera
        QString rightCameraPrefix = "RightCamera";

        QStringList patternParameters;
        patternParameters << "PatternWidth" << "PatternHeight" << "ElementSize" << "PatternType" << "ScaleLevels" << "ScaleIncrement";

        for (const QString &fieldName : patternParameters) {
            setField(fieldPrefix + fieldName, field(rightCameraPrefix + fieldName));
        }
    }
}


QStringList PageStereoImages::getImages () const
{
    QStringList images;

    if (radioButtonInterleaved->isChecked()) {
        // Interleaved; we can use parent's function
        images = PageImages::getImages();
    } else {
        // Grouped; interleave them ourselves
        int numPairs = listWidgetImages->count() / 2;

        images.reserve(numPairs*2);
        for (int i = 0; i < numPairs; i++) {
            images.append(listWidgetImages->item(i)->text());
            images.append(listWidgetImages->item(numPairs + i)->text());
        }
    }

    return images;
}


void PageStereoImages::setImagesOrder (int order)
{
    if (order == Interleaved) {
        radioButtonInterleaved->setChecked(true);
    } else {
        radioButtonGrouped->setChecked(true);
    }

    updateImageEntries();
}

int PageStereoImages::getImagesOrder () const
{
    if (radioButtonInterleaved->isChecked()) {
        return Interleaved;
    } else {
        return Grouped;
    }
}

void PageStereoImages::updateImageEntries ()
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


} // CalibrationWizard
} // GUI
} // StereoToolbox
} // MVL
