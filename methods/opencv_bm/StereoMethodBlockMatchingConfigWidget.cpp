/*
 * OpenCV Block Matching: config widget
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

#include "StereoMethodBlockMatchingConfigWidget.h"
#include "StereoMethodBlockMatching.h"


StereoMethodBlockMatchingConfigWidget::StereoMethodBlockMatchingConfigWidget (StereoMethodBlockMatching *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QFrame *line;
    QCheckBox *checkBox;

    QString tooltip;

    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Name
    label = new QLabel("<b><u>OpenCV block matching</u></b>", this);
    label->setAlignment(Qt::AlignHCenter);

    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Preset
    tooltip = "Presets for quick initialization.";
    
    label = new QLabel("Preset", this);
    label->setToolTip(tooltip);

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
    comboBoxPreset = comboBox;

    layout->addRow(label, comboBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Pre-filter type
    tooltip = "Type of pre-filter used for normalizing input images.";

    label = new QLabel("Pre-filter type", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("NORMALIZED_RESPONSE", CV_STEREO_BM_NORMALIZED_RESPONSE);
    comboBox->setItemData(0, "Normalized response filter.", Qt::ToolTipRole);
    comboBox->addItem("XSOBEL", CV_STEREO_BM_XSOBEL);
    comboBox->setItemData(0, "Sobel filter.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(preFilterTypeChanged(int)));
    comboBoxPreFilterType = comboBox;

    layout->addRow(label, comboBox);

    // Pre-filter size
    tooltip = "Pre-filter size; ~5x5 ... 21x21";

    label = new QLabel("Pre-filter size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(5, 255); // 5...255
    spinBox->setSingleStep(2); // Allows only odd values
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPreFilterSize(int)));
    spinBoxPreFilterSize = spinBox;

    layout->addRow(label, spinBox);

    // Pre-filter cap
    tooltip = "Truncation value for the pre-filtered image pixels; up to ~31.";

    label = new QLabel("Pre-filter cap", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 63);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPreFilterCap(int)));
    spinBoxPreFilterCap = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // SAD window size
    tooltip = "Window size for Sum of Absolute Difference (SAD): the linear size of the blocks \n"
              "compared by the algorithm. The size should be odd (as the block is centered at the \n"
              "current pixel). Larger block size implies smoother, though less accurate disparity map. \n"
              "Smaller block size gives more detailed disparity map, but there is higher chance for \n"
              "algorithm to find a wrong correspondence.";

    label = new QLabel("SAD window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(5, 255); // 5...255
    spinBox->setSingleStep(2); // Always odd values
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSADWindowSize(int)));
    spinBoxSADWindowSize = spinBox;

    layout->addRow(label, spinBox);

    // Min disparity
    tooltip = "Minimum possible disparity value. Normally, it is zero but sometimes rectification \n"
              "algorithms can shift images, so this parameter needs to be adjusted accordingly.";

    label = new QLabel("Min. disparity", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-9999, 9999);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMinDisparity(int)));
    spinBoxMinDisparity = spinBox;

    layout->addRow(label, spinBox);

    // Num disparities
    tooltip = "Maximum disparity minus minimum disparity. The value is always greater than zero. In \n"
              "the current implementation, this parameter must be divisible by 16.";
    
    label = new QLabel("Num. disparities", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(16, 16*1000);
    spinBox->setSingleStep(16); // Must be divisible by 16
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumDisparities(int)));
    spinBoxNumDisparities = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Texture threshold
    tooltip = "Texture threshold; areas with no texture (or texture below threshold) are ignored.";
    
    label = new QLabel("Texture threshold", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 32000);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setTextureThreshold(int)));
    spinBoxTextureThreshold = spinBox;

    layout->addRow(label, spinBox);

    // Uniqueness ratio
    tooltip = "Filter out pixels if there are other close with different disparity, as controlled by this ratio.";

    label = new QLabel("Uniqueness ratio", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 255);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setUniquenessRatio(int)));
    spinBoxUniquenessRatio = spinBox;

    layout->addRow(label, spinBox);

    // Speckle window size
    tooltip = "The maximum area of speckles to remove (set to 0 to disable speckle filtering).";

    label = new QLabel("Speckle window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 200);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleWindowSize(int)));
    spinBoxSpeckleWindowSize = spinBox;

    layout->addRow(label, spinBox);

    // Speckle range
    tooltip = "Acceptable range of disparity variation in each connected component.";
    
    label = new QLabel("Speckle range", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 100);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleRange(int)));
    spinBoxSpeckleRange = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Try smaller windows
    tooltip = "If enabled, results may be more accurate, at the expense of slower processing.";
    
    checkBox = new QCheckBox("Try smaller windows", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(trySmallerWindowsChanged(int)));
    checkBoxTrySmallerWindow = checkBox;

    layout->addRow(checkBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Disp12MaxDiff
    tooltip = "Maximum allowed difference (in integer pixel units) in the left-right disparity check. \n"
              "Set it to a non-positive value to disable the check.";

    label = new QLabel("Disp12MaxDiff", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-1, 255);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setDisp12MaxDiff(int)));
    spinBoxDisp12MaxDiff = spinBox;

    layout->addRow(label, spinBox);

    // Update parameters
    updateParameters();
}

StereoMethodBlockMatchingConfigWidget::~StereoMethodBlockMatchingConfigWidget ()
{
}

void StereoMethodBlockMatchingConfigWidget::presetChanged (int index)
{
    method->usePreset(comboBoxPreset->itemData(index).toInt());
}

void StereoMethodBlockMatchingConfigWidget::preFilterTypeChanged (int index)
{
    method->setPreFilterType(comboBoxPreFilterType->itemData(index).toInt());
}

void StereoMethodBlockMatchingConfigWidget::trySmallerWindowsChanged (int state)
{
    method->setTrySmallerWindows(state == Qt::Checked);
}


void StereoMethodBlockMatchingConfigWidget::updateParameters ()
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
