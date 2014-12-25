/*
 * OpenCV Block Matching: config widget
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

#include "method_widget.h"
#include "method.h"

using namespace StereoMethodBlockMatching;


MethodWidget::MethodWidget (Method *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QFrame *line;

    QString tooltip;

    // Name
    label = new QLabel("<b><u>OpenCV block matching</u></b>", this);
    label->setAlignment(Qt::AlignHCenter);

    baseLayout->addWidget(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    baseLayout->addWidget(line);

    // Scrollable area with layout
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(new QWidget(this));

    baseLayout->addWidget(scrollArea);
    
    QFormLayout *layout = new QFormLayout(scrollArea->widget());


    // Preset
    tooltip = "Presets for quick initialization.";
    
    label = new QLabel("Preset", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("OpenCV", Method::OpenCV);
    comboBox->setItemData(0, "Initial OpenCV settings.", Qt::ToolTipRole);
    comboBox->addItem("StereoMatch", Method::StereoMatch);
    comboBox->setItemData(1, "Settings from \"Stereo Match\" example.", Qt::ToolTipRole);
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
    comboBox->addItem("NORMALIZED_RESPONSE", cv::StereoBM::PREFILTER_NORMALIZED_RESPONSE);
    comboBox->setItemData(0, "Normalized response filter.", Qt::ToolTipRole);
    comboBox->addItem("XSOBEL", cv::StereoBM::PREFILTER_XSOBEL);
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

MethodWidget::~MethodWidget ()
{
}

void MethodWidget::presetChanged (int index)
{
    method->usePreset(comboBoxPreset->itemData(index).toInt());
}

void MethodWidget::preFilterTypeChanged (int index)
{
    method->setPreFilterType(comboBoxPreFilterType->itemData(index).toInt());
}

void MethodWidget::updateParameters ()
{
    bool oldState;

    // Pre-filter type
    oldState = comboBoxPreFilterType->blockSignals(true);
    comboBoxPreFilterType->setCurrentIndex(comboBoxPreFilterType->findData(method->getPreFilterType()));
    comboBoxPreFilterType->blockSignals(oldState);

    // Pre-filter size
    oldState = spinBoxPreFilterSize->blockSignals(true);
    spinBoxPreFilterSize->setValue(method->getPreFilterSize());
    spinBoxPreFilterSize->blockSignals(oldState);

    // Pre-filter cap
    oldState = spinBoxPreFilterCap->blockSignals(true);
    spinBoxPreFilterCap->setValue(method->getPreFilterCap());
    spinBoxPreFilterCap->blockSignals(oldState);
    

    // SAD window size
    oldState = spinBoxSADWindowSize->blockSignals(true);
    spinBoxSADWindowSize->setValue(method->getSADWindowSize());
    spinBoxSADWindowSize->blockSignals(oldState);

    // Min. disparity
    oldState = spinBoxMinDisparity->blockSignals(true);
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxMinDisparity->blockSignals(oldState);

    // Num. disparities
    oldState = spinBoxNumDisparities->blockSignals(true);
    spinBoxNumDisparities->setValue(method->getNumDisparities());
    spinBoxNumDisparities->blockSignals(oldState);
    

    // Texture threshold
    oldState = spinBoxTextureThreshold->blockSignals(true);
    spinBoxTextureThreshold->setValue(method->getTextureThreshold());
    spinBoxTextureThreshold->blockSignals(oldState);

    // Uniqueness ratio
    oldState = spinBoxUniquenessRatio->blockSignals(true);
    spinBoxUniquenessRatio->setValue(method->getUniquenessRatio());
    spinBoxUniquenessRatio->blockSignals(oldState);

    // Speckle window size
    oldState = spinBoxSpeckleWindowSize->blockSignals(true);
    spinBoxSpeckleWindowSize->setValue(method->getSpeckleWindowSize());
    spinBoxSpeckleWindowSize->blockSignals(oldState);

    // Speckle range
    oldState = spinBoxSpeckleRange->blockSignals(true);
    spinBoxSpeckleRange->setValue(method->getSpeckleRange());
    spinBoxSpeckleRange->blockSignals(oldState);
    
    // Disp12 max diff
    oldState = spinBoxDisp12MaxDiff->blockSignals(true);
    spinBoxDisp12MaxDiff->setValue(method->getDisp12MaxDiff());
    spinBoxDisp12MaxDiff->blockSignals(oldState);
}
