/*
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
 
#include "StereoMethodBlockMatching.h"

#include <opencv2/imgproc/imgproc.hpp>


StereoMethodBlockMatching::StereoMethodBlockMatching (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "BM";
    configWidget = new ConfigTabBlockMatching(this);

    usePreset(OpenCVBasic);
}

StereoMethodBlockMatching::~StereoMethodBlockMatching ()
{
    //delete configWidget;
}


void StereoMethodBlockMatching::usePreset (int type)
{
    switch (type) {
        case OpenCVBasic: {
            // OpenCV basic
            bm = cv::StereoBM(cv::StereoBM::BASIC_PRESET);
            break;
        }
        case OpenCVFishEye: {
            // OpenCV fish eye
            bm = cv::StereoBM(cv::StereoBM::FISH_EYE_PRESET);
            break;
        }
        case OpenCVNarrow: {
            // OpenCV narrow
            bm = cv::StereoBM(cv::StereoBM::NARROW_PRESET);
            break;
        }
        case StereoMatch: {
            // "Stereo match" example
            bm = cv::StereoBM();

            bm.state->preFilterCap = 31;
            bm.state->SADWindowSize = 9;
            bm.state->minDisparity = 0;
            bm.state->numberOfDisparities = ((imageWidth/8) + 15) & -16;
            bm.state->textureThreshold = 10;
            bm.state->uniquenessRatio = 15;
            bm.state->speckleWindowSize = 100;
            bm.state->speckleRange = 32;
            bm.state->disp12MaxDiff = 1;
            
            break;
        }
    };

    emit parameterChanged();
}

// *********************************************************************
// *                      Depth image computation                      *
// *********************************************************************
void StereoMethodBlockMatching::computeDepthImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &depth)
{
    // Convert to grayscale
    if (img1.channels() == 3) {
        cv::cvtColor(img1, tmpImg1, CV_RGB2GRAY);
    } else {
        tmpImg1 = img1;
    }

    if (img2.channels() == 3) {
        cv::cvtColor(img2, tmpImg2, CV_RGB2GRAY);
    } else {
        tmpImg2 = img2;
    }

    
    // Compute depth image
    bm(tmpImg1, tmpImg2, tmpDepth);

    // Normalize to output
    tmpDepth.convertTo(depth, CV_8U, 255/(bm.state->numberOfDisparities*16.));
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Pre-filter type
int StereoMethodBlockMatching::getPreFilterType () const
{
    return bm.state->preFilterType;
}

void StereoMethodBlockMatching::setPreFilterType (int newValue)
{
    // Validate
    if (newValue != CV_STEREO_BM_NORMALIZED_RESPONSE && newValue != CV_STEREO_BM_XSOBEL) {
        newValue = CV_STEREO_BM_NORMALIZED_RESPONSE;
    }

    setParameter(bm.state->preFilterType, newValue);
}

// Pre-filter size
int StereoMethodBlockMatching::getPreFilterSize () const
{
    return bm.state->preFilterSize;
}

void StereoMethodBlockMatching::setPreFilterSize (int newValue)
{
    // Validate
    newValue += !(newValue % 2); // Must be odd
    newValue = qBound(5, newValue, 255);

    setParameter(bm.state->preFilterSize, newValue);
}
    
// Pre-filter clipping
int StereoMethodBlockMatching::getPreFilterCap () const
{
    return bm.state->preFilterCap;
}

void StereoMethodBlockMatching::setPreFilterCap (int newValue)
{
    // Validate
    newValue = qBound(1, newValue, 63);

    setParameter(bm.state->preFilterCap, newValue);
}
        

// Sum-of-absolute difference window size
int StereoMethodBlockMatching::getSADWindowSize () const
{
    return bm.state->SADWindowSize;
}

void StereoMethodBlockMatching::setSADWindowSize (int newValue)
{
    // Validate
    newValue += !(newValue % 2); // Must be odd
    newValue = qBound(5, newValue, 255);

    setParameter(bm.state->SADWindowSize, newValue);
}
    
// Minimum disparity
int StereoMethodBlockMatching::getMinDisparity () const
{
    return bm.state->minDisparity;
}

void StereoMethodBlockMatching::setMinDisparity (int newValue)
{
    setParameter(bm.state->minDisparity, newValue);
}

// Number of disparity levels
int StereoMethodBlockMatching::getNumDisparities () const
{
    return bm.state->numberOfDisparities;
}

void StereoMethodBlockMatching::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 16.0) * 16; // Must be divisible by 16
    newValue = qMax(16, newValue);
    
    setParameter(bm.state->numberOfDisparities, newValue);
}
    
  
// Post-filtering texture threshold
int StereoMethodBlockMatching::getTextureThreshold () const
{
    return bm.state->textureThreshold;
}

void StereoMethodBlockMatching::setTextureThreshold (int newValue)
{
    setParameter(bm.state->textureThreshold, newValue);
}

// Uniqueness ratio; accept disparity d* only if:
//  SAD(d) >= SAD(d*)*(1 + uniquenessRatio/100.)
// for any d!) d* +/- 1 within the search range
int StereoMethodBlockMatching::getUniquenessRatio () const
{
    return bm.state->uniquenessRatio;
}

void StereoMethodBlockMatching::setUniquenessRatio (int newValue)
{
    setParameter(bm.state->uniquenessRatio, newValue);
}

// Disparity variantion window
int StereoMethodBlockMatching::getSpeckleWindowSize () const
{
    return bm.state->speckleWindowSize;
}

void StereoMethodBlockMatching::setSpeckleWindowSize (int newValue)
{
    setParameter(bm.state->speckleWindowSize, newValue);
}

// Acceptable range of variation in window    
int StereoMethodBlockMatching::getSpeckleRange () const
{
    return bm.state->speckleRange;
}

void StereoMethodBlockMatching::setSpeckleRange (int newValue)
{
    setParameter(bm.state->speckleRange, newValue);
}
 
// Whether to try smaller windows or not (more accurate results, but slower)
bool StereoMethodBlockMatching::getTrySmallerWindows () const
{
    return bm.state->trySmallerWindows;
}

void StereoMethodBlockMatching::setTrySmallerWindows (bool newValue)
{
    setParameter(bm.state->trySmallerWindows, (int)newValue);
}

// Disp12MaxDiff
int StereoMethodBlockMatching::getDisp12MaxDiff () const
{
    return bm.state->disp12MaxDiff;
}

void StereoMethodBlockMatching::setDisp12MaxDiff (int newValue)
{
    setParameter(bm.state->disp12MaxDiff, newValue);
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabBlockMatching::ConfigTabBlockMatching (StereoMethodBlockMatching *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    QLabel *label;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QFrame *line;
    QCheckBox *checkBox;

    QString tooltip;

    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Name
    label = new QLabel("OpenCV block matching", this);
    label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(label, row, 0, 1, 2);

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Preset
    tooltip = "Presets for quick initialization.";
    
    label = new QLabel("Preset", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    comboBox = new QComboBox(this);
    comboBox->addItem("OpenCV - Basic", StereoMethodBlockMatching::OpenCVBasic);
    comboBox->setItemData(0, "Initial OpenCV settings with \"Basic\" preset.", Qt::ToolTipRole);
    comboBox->addItem("OpenCV - FishEye", StereoMethodBlockMatching::OpenCVFishEye);
    comboBox->setItemData(1, "Initial OpenCV settings with \"FishEye\" preset.", Qt::ToolTipRole);
    comboBox->addItem("OpenCV - Narrow", StereoMethodBlockMatching::OpenCVNarrow);
    comboBox->setItemData(2, "Initial OpenCV settings with \"Narrow\" preset.", Qt::ToolTipRole);
    comboBox->addItem("StereoMatch", StereoMethodBlockMatching::StereoMatch);
    comboBox->setItemData(3, "Settings from \"Stereo Match\" example.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(presetChanged(int)));
    layout->addWidget(comboBox, row, 1);
    comboBoxPreset = comboBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Pre-filter type
    tooltip = "Type of pre-filter used for normalizing input images.";

    label = new QLabel("Pre-filter type", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    comboBox = new QComboBox(this);
    comboBox->addItem("NORMALIZED_RESPONSE", CV_STEREO_BM_NORMALIZED_RESPONSE);
    comboBox->setItemData(0, "Normalized response filter.", Qt::ToolTipRole);
    comboBox->addItem("XSOBEL", CV_STEREO_BM_XSOBEL);
    comboBox->setItemData(0, "Sobel filter.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(preFilterTypeChanged(int)));
    layout->addWidget(comboBox, row, 1);
    comboBoxPreFilterType = comboBox;

    row++;

    // Pre-filter size
    tooltip = "Pre-filter size; ~5x5 ... 21x21";

    label = new QLabel("Pre-filter size", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(5, 255); // 5...255
    spinBox->setSingleStep(2); // Allows only odd values
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPreFilterSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxPreFilterSize = spinBox;

    row++;

    // Pre-filter cap
    tooltip = "Truncation value for the pre-filtered image pixels; up to ~31.";

    label = new QLabel("Pre-filter cap", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 63);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPreFilterCap(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxPreFilterCap = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // SAD window size
    tooltip = "Window size for Sum of Absolute Difference (SAD): the linear size of the blocks \n"
              "compared by the algorithm. The size should be odd (as the block is centered at the \n"
              "current pixel). Larger block size implies smoother, though less accurate disparity map. \n"
              "Smaller block size gives more detailed disparity map, but there is higher chance for \n"
              "algorithm to find a wrong correspondence.";

    label = new QLabel("SAD window size", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(5, 255); // 5...255
    spinBox->setSingleStep(2); // Always odd values
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSADWindowSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSADWindowSize = spinBox;

    row++;

    // Min disparity
    tooltip = "Minimum possible disparity value. Normally, it is zero but sometimes rectification \n"
              "algorithms can shift images, so this parameter needs to be adjusted accordingly.";

    label = new QLabel("Min. disparity", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-9999, 9999);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMinDisparity(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxMinDisparity = spinBox;

    row++;

    // Num disparities
    tooltip = "Maximum disparity minus minimum disparity. The value is always greater than zero. In \n"
              "the current implementation, this parameter must be divisible by 16.";
    
    label = new QLabel("Num. disparities", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(16, 16*1000);
    spinBox->setSingleStep(16); // Must be divisible by 16
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumDisparities(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxNumDisparities = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Texture threshold
    tooltip = "Texture threshold; areas with no texture (or texture below threshold) are ignored.";
    
    label = new QLabel("Texture threshold", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 32000);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setTextureThreshold(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxTextureThreshold = spinBox;

    row++;

    // Uniqueness ratio
    tooltip = "Filter out pixels if there are other close with different disparity, as controlled by this ratio.";

    label = new QLabel("Uniqueness ratio", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 255);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setUniquenessRatio(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxUniquenessRatio = spinBox;

    row++;

    // Speckle window size
    tooltip = "The maximum area of speckles to remove (set to 0 to disable speckle filtering).";

    label = new QLabel("Speckle window size", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 200);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleWindowSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSpeckleWindowSize = spinBox;

    row++;

    // Speckle range
    tooltip = "Acceptable range of disparity variation in each connected component.";
    
    label = new QLabel("Speckle range", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 100);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleRange(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSpeckleRange = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Try smaller windows
    tooltip = "If enabled, results may be more accurate, at the expense of slower processing.";
    
    checkBox = new QCheckBox("Try smaller windows", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(trySmallerWindowsChanged(int)));
    layout->addWidget(checkBox, row, 0, 1, 2);
    checkBoxTrySmallerWindow = checkBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Disp12MaxDiff
    tooltip = "Maximum allowed difference (in integer pixel units) in the left-right disparity check. \n"
              "Set it to a non-positive value to disable the check.";

    label = new QLabel("Disp12MaxDiff", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-1, 255);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setDisp12MaxDiff(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxDisp12MaxDiff = spinBox;

    row++;

    // Spacer for padding
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, 0, 1, 2);

    // Update parameters
    updateParameters();
}

ConfigTabBlockMatching::~ConfigTabBlockMatching ()
{
}

void ConfigTabBlockMatching::presetChanged (int index)
{
    method->usePreset(comboBoxPreset->itemData(index).toInt());
}

void ConfigTabBlockMatching::preFilterTypeChanged (int index)
{
    method->setPreFilterType(comboBoxPreFilterType->itemData(index).toInt());
}

void ConfigTabBlockMatching::trySmallerWindowsChanged (int state)
{
    method->setTrySmallerWindows(state == Qt::Checked);
}


void ConfigTabBlockMatching::updateParameters ()
{
    comboBoxPreFilterType->setCurrentIndex(comboBoxPreFilterType->findData(method->getPreFilterType()));
    
    spinBoxPreFilterSize->setValue(method->getPreFilterSize());
    spinBoxPreFilterCap->setValue(method->getPreFilterCap());
    
    spinBoxSADWindowSize->setValue(method->getSADWindowSize());
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxNumDisparities->setValue(method->getNumDisparities());
    
    spinBoxTextureThreshold->setValue(method->getTextureThreshold());
    spinBoxUniquenessRatio->setValue(method->getUniquenessRatio());
    spinBoxSpeckleWindowSize->setValue(method->getSpeckleWindowSize());
    spinBoxSpeckleRange->setValue(method->getSpeckleRange());
    
    checkBoxTrySmallerWindow->setChecked(method->getTrySmallerWindows());

    spinBoxDisp12MaxDiff->setValue(method->getDisp12MaxDiff());
}
