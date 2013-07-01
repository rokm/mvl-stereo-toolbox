/*
 * OpenCV Semi-Global Block Matching: config widget
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
 
#include "StereoMethodSemiGlobalBlockMatchingConfigWidget.h"
#include "StereoMethodSemiGlobalBlockMatching.h"


StereoMethodSemiGlobalBlockMatchingConfigWidget::StereoMethodSemiGlobalBlockMatchingConfigWidget (StereoMethodSemiGlobalBlockMatching *m, QWidget *parent)
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
    label = new QLabel("<b><u>OpenCV semi-global block matching</u></b>", this);
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
    comboBox->addItem("OpenCV", StereoMethodSemiGlobalBlockMatching::OpenCV);
    comboBox->setItemData(0, "Initial OpenCV settings.", Qt::ToolTipRole);
    comboBox->addItem("StereoMatch", StereoMethodSemiGlobalBlockMatching::StereoMatch);
    comboBox->setItemData(1, "Settings from \"Stereo Match\" example.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(presetChanged(int)));
    comboBoxPreset = comboBox;

    layout->addRow(label, comboBox);

    // Min disparity
    tooltip = "Minimum possible disparity value. Normally, it is zero but sometimes rectification algorithms \n"
              "can shift images, so this parameter needs to be adjusted accordingly.";
              
    label = new QLabel("Min. disparity", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMinDisparity(int)));
    spinBoxMinDisparity = spinBox;

    layout->addRow(label, spinBox);

    // Num disparities
    tooltip = "Maximum disparity minus minimum disparity. The value is always greater than zero. In the current \n"
              "implementation, this parameter must be divisible by 16.";
              
    label = new QLabel("Num. disparities", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(16, 16*1000);
    spinBox->setSingleStep(16); // Must be divisible by 16
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumDisparities(int)));
    spinBoxNumDisparities = spinBox;

    layout->addRow(label, spinBox);

    // SAD window size
    tooltip = "Matched block size. It must be an odd number >=1. Normally, it should be somewhere in the 3-11 range.";
    
    label = new QLabel("SAD window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 255);
    spinBox->setSingleStep(1); // Always odd values
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSADWindowSize(int)));
    spinBoxSADWindowSize = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Pre-filter cap
    tooltip = "Truncation value for the prefiltered image pixels. The algorithm first computes x-derivative at each pixel \n"
              "and clips its value by [-preFilterCap, preFilterCap] interval. The result values are passed to the \n"
              "Birchfield-Tomasi pixel cost function.";
              
    label = new QLabel("Pre-filter cap", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 63);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPreFilterCap(int)));
    spinBoxPreFilterCap = spinBox;

    layout->addRow(label, spinBox);

    // Uniqueness ratio
    tooltip = "Margin in percentage by which the best (minimum) computed cost function value should \"win\" the second best \n"
              "value to consider the found match correct. Normally, a value within the 5-15 range is good enough.";
    
    label = new QLabel("Uniqueness ratio", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 255);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setUniquenessRatio(int)));
    spinBoxUniquenessRatio = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // P1
    tooltip = "The first parameter controlling the disparity smoothness. The larger the \n"
              "value is, the smoother the disparity is. P1 is the penalty on the disparity \n"
              "change by plus or minus 1 between neighbor pixels. Algorithm requires P2 > P1. \n"
              "OpenCV stereo example recommends multipliers of numImageChannels*SADWindowSize^2";
              
    label = new QLabel("P1", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setP1(int)));
    spinBoxP1 = spinBox;

    layout->addRow(label, spinBox);

    // P2
    tooltip = "The second parameter controlling the disparity smoothness. The larger the \n"
              "value is, the smoother the disparity is. P2 is the penalty on the disparity \n"
              "change by more than 1 between neighbor pixels. The algorithm requires P2 > P1. \n"
              "OpenCV stereo example recommends multipliers of numImageChannels*SADWindowSize^2";
              
    label = new QLabel("P2", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setP2(int)));
    spinBoxP2 = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Speckle window size
    tooltip = "Maximum size of smooth disparity regions to consider their noise speckles and invalidate. \n"
              "Set it to 0 to disable speckle filtering. Otherwise, set it somewhere in the 50-200 range.";
              
    label = new QLabel("Speckle window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 100);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleWindowSize(int)));
    spinBoxSpeckleWindowSize = spinBox;

    layout->addRow(label, spinBox);

    // Speckle range
    tooltip = "Maximum disparity variation within each connected component. If you do speckle filtering, set \n"
              "the parameter to a positive value, it will be implicitly multiplied by 16. Normally, 1 or 2 is \n"
              "good enough.";
              
    label = new QLabel("Speckle range", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 200);
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
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setDisp12MaxDiff(int)));
    spinBoxDisp12MaxDiff = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Full DP
    tooltip = "Set it to true to run the full-scale two-pass dynamic programming algorithm. It will consume \n"
              "O(W*H*numDisparities) bytes, which is large for 640x480 stereo and huge for HD-size pictures.";
    
    checkBox = new QCheckBox("Full DP", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(fullDPChanged(int)));
    checkBoxFullDP = checkBox;

    layout->addRow(checkBox);

    // Update parameters
    updateParameters();
}

StereoMethodSemiGlobalBlockMatchingConfigWidget::~StereoMethodSemiGlobalBlockMatchingConfigWidget ()
{
}

void StereoMethodSemiGlobalBlockMatchingConfigWidget::presetChanged (int index)
{
    method->usePreset(comboBoxPreset->itemData(index).toInt());
}

void StereoMethodSemiGlobalBlockMatchingConfigWidget::fullDPChanged (int state)
{
    method->setFullDP(state == Qt::Checked);
}


void StereoMethodSemiGlobalBlockMatchingConfigWidget::updateParameters ()
{
    bool oldState;

    // Min. disparity
    oldState = spinBoxMinDisparity->blockSignals(true);
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxMinDisparity->blockSignals(oldState);

    // Num. disparities
    oldState = spinBoxNumDisparities->blockSignals(true);
    spinBoxNumDisparities->setValue(method->getNumDisparities());
    spinBoxNumDisparities->blockSignals(oldState);

    // SAD window size
    oldState = spinBoxSADWindowSize->blockSignals(true);
    spinBoxSADWindowSize->setValue(method->getSADWindowSize());
    spinBoxSADWindowSize->blockSignals(oldState);


    // Pre-filter cap
    oldState = spinBoxPreFilterCap->blockSignals(true);
    spinBoxPreFilterCap->setValue(method->getPreFilterCap());
    spinBoxPreFilterCap->blockSignals(oldState);
    

    // Uniqueness ratio
    oldState = spinBoxUniquenessRatio->blockSignals(true);
    spinBoxUniquenessRatio->setValue(method->getUniquenessRatio());
    spinBoxUniquenessRatio->blockSignals(oldState);

    // P1 and P2 require some more work...
    int SADWindowSize = method->getSADWindowSize();
    int numChannels = method->getImageChannels();
    if (SADWindowSize > 0 && numChannels > 0) {
        // P1
        oldState = spinBoxP1->blockSignals(true);
        spinBoxP1->setValue(method->getP1() / (numChannels*SADWindowSize*SADWindowSize));
        spinBoxP1->blockSignals(oldState);

        // P2
        oldState = spinBoxP2->blockSignals(true);
        spinBoxP2->setValue(method->getP2() / (numChannels*SADWindowSize*SADWindowSize));
        spinBoxP2->blockSignals(oldState);
    } else {
        // P1
        oldState = spinBoxP1->blockSignals(true);
        spinBoxP1->setValue(method->getP1());
        spinBoxP1->blockSignals(oldState);

        // P2
        oldState = spinBoxP2->blockSignals(true);
        spinBoxP2->setValue(method->getP2());
        spinBoxP2->blockSignals(oldState);
    }

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


    // Full DP
    oldState = checkBoxFullDP->blockSignals(true);
    checkBoxFullDP->setChecked(method->getFullDP());
    checkBoxFullDP->blockSignals(oldState);
}
