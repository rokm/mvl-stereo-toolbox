/*
 * Efficient LArge-scale Stereo: config widget
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

using namespace StereoMethodELAS;


MethodWidget::MethodWidget (Method *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QDoubleSpinBox *spinBoxD;
    QFrame *line;
    QCheckBox *checkBox;

    QString tooltip;

    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Name
    label = new QLabel("<b><u>Efficient LArge-scale Stereo</u></b>", this);
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
    comboBox->addItem("Robotics", Method::ElasRobotics);
    comboBox->setItemData(0, "Default settings in robotics environment. Does not produce results in \nhalf-occluded areas and is a bit more robust to lighting, etc.", Qt::ToolTipRole);
    comboBox->addItem("Middlebury", Method::ElasMiddlebury);
    comboBox->setItemData(1, "Default settings for Middlebury benchmark. Interpolates all missing disparities.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(presetChanged(int)));
    comboBoxPreset = comboBox;

    layout->addRow(label, comboBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Min disparity
    tooltip = "Minimum possible disparity value.";

    label = new QLabel("Min. disparity", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMinDisparity(int)));
    spinBoxMinDisparity = spinBox;

    layout->addRow(label, spinBox);

    // Max disparity
    tooltip = "Maximum possible disparity value.";

    label = new QLabel("Max. disparity", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMaxDisparity(int)));
    spinBoxMaxDisparity = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Support threshold
    tooltip = "Max. uniqueness ratio (best vs. second best support match).";

    label = new QLabel("Support threshold", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setSupportThreshold(double)));
    spinBoxSupportThreshold = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Support texture
    tooltip = "Min. texture for support points.";

    label = new QLabel("Support texture", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSupportTexture(int)));
    spinBoxSupportTexture = spinBox;

    layout->addRow(label, spinBox);

    // Candidate step size
    tooltip = "Step size of regular grid on which support points are matched.";

    label = new QLabel("Candidate step size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setCandidateStepSize(int)));
    spinBoxCandidateStepSize = spinBox;

    layout->addRow(label, spinBox);

    // Inconsistent window size
    tooltip = "Window size of inconsistent support point check";

    label = new QLabel("Incon. window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setInconsistentWindowSize(int)));
    spinBoxInconsistentWindowSize = spinBox;

    layout->addRow(label, spinBox);

    // Inconsistent threshold
    tooltip = "Disparity similarity threshold for support point to be considered consistent.";

    label = new QLabel("Incon. threshold", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setInconsistentThreshold(int)));
    spinBoxInconsistentThreshold = spinBox;

    layout->addRow(label, spinBox);

    // Inconsistent min support
    tooltip = "Minimum number of consistent support points.";

    label = new QLabel("Incon. min. support", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setInconsistentMinSupport(int)));
    spinBoxInconsistentMinSupport = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Add corners
    tooltip = "Add support points at image corners with nearest neighbor disparities.";
    
    checkBox = new QCheckBox("Add corners", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(addCornersChanged(int)));
    checkBoxAddCorners = checkBox;

    layout->addRow(checkBox);

    // Grid size
    tooltip = "Size of neighborhood for additional support point extrapolation.";

    label = new QLabel("Grid size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setGridSize(int)));
    spinBoxGridSize = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);
    
    // Beta
    tooltip = "Image likelihood parameter.";

    label = new QLabel("Beta", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setBeta(double)));
    spinBoxBeta = spinBoxD;

    layout->addRow(label, spinBoxD);
    
    // Gamma
    tooltip = "Prior constant.";

    label = new QLabel("Gamma", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setGamma(double)));
    spinBoxGamma = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Sigma
    tooltip = "Prior sigma.";

    label = new QLabel("Sigma", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setSigma(double)));
    spinBoxSigma = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Sigma radius
    tooltip = "Prior sigma radius.";

    label = new QLabel("Sigma radius", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setSigmaRadius(double)));
    spinBoxSigmaRadius = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Match texture
    tooltip = "Min texture for dense matching.";

    label = new QLabel("Match texture", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMatchTexture(int)));
    spinBoxMatchTexture = spinBox;

    layout->addRow(label, spinBox);

    // Left/right threshold
    tooltip = "Disparity threshold for left/right consistency check.";

    label = new QLabel("Left/right thr.", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setLRThreshold(int)));
    spinBoxLRThreshold = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Speckle simimarity threshold
    tooltip = "Similarity threshold for speckle segmentation.";

    label = new QLabel("Speckle sim. thr.", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setSpeckleSimThreshold(double)));
    spinBoxSpeckleSimThreshold = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Speckle size
    tooltip = "Maximal size of a speckle (small speckles are removed).";

    label = new QLabel("Speckle size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleSize(int)));
    spinBoxSpeckleSize = spinBox;

    layout->addRow(label, spinBox);

    // Interpolation gap width
    tooltip = "Interpolate small gaps (left<->right, top<->bottom).";

    label = new QLabel("Interp. gap width", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setInterpolationGapWidth(int)));
    spinBoxInterpolationGapWidth = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Median filter
    tooltip = "Optional median filter (approximated).";
    
    checkBox = new QCheckBox("Median filter", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(medianFilterChanged(int)));
    checkBoxFilterMedian = checkBox;

    layout->addRow(checkBox);

    // Adaptive mean filter
    tooltip = "Optional adaptive mean filter (approximated).";
    
    checkBox = new QCheckBox("Adaptive mean filter", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(adaptiveMeanFilterChanged(int)));
    checkBoxFilterAdaptiveMean = checkBox;

    layout->addRow(checkBox);

    // Postprocess only left
    tooltip = "Save time by not post-processing the right disparity image.";
    
    checkBox = new QCheckBox("Post-process only left", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(postProcessOnlyLeftChanged(int)));
    checkBoxPostProcessOnlyLeft = checkBox;

    layout->addRow(checkBox);

    // Subsampling
    tooltip = "Save time by only computing disparities for each 2nd pixel.";
    
    checkBox = new QCheckBox("Subsampling", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(subsamplingChanged(int)));
    checkBoxSubsampling = checkBox;

    layout->addRow(checkBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Return left
    tooltip = "If checked, method returns left disparity image. If unchecked, right disparity image is returned.";
    
    checkBox = new QCheckBox("Return left disp. image", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(returnLeftChanged(int)));
    checkBoxReturnLeft = checkBox;

    layout->addRow(checkBox);

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

void MethodWidget::addCornersChanged (int state)
{
    method->setAddCorners(state == Qt::Checked);
}

void MethodWidget::medianFilterChanged (int state)
{
    method->setFilterMedian(state == Qt::Checked);
}

void MethodWidget::adaptiveMeanFilterChanged (int state)
{
    method->setFilterAdaptiveMean(state == Qt::Checked);
}

void MethodWidget::postProcessOnlyLeftChanged (int state)
{
    method->setPostProcessOnlyLeft(state == Qt::Checked);
}

void MethodWidget::subsamplingChanged (int state)
{
    method->setSubsampling(state == Qt::Checked);
}

void MethodWidget::returnLeftChanged (int state)
{
    method->setReturnLeft(state == Qt::Checked);
}

void MethodWidget::updateParameters ()
{
    bool oldState;

    // Min disparity
    oldState = spinBoxMinDisparity->blockSignals(true);
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxMinDisparity->blockSignals(oldState);
    
    // Max disparity
    oldState = spinBoxMaxDisparity->blockSignals(true);
    spinBoxMaxDisparity->setValue(method->getMaxDisparity());
    spinBoxMaxDisparity->blockSignals(oldState);


    // Support threshold
    oldState = spinBoxSupportThreshold->blockSignals(true);
    spinBoxSupportThreshold->setValue(method->getSupportThreshold());
    spinBoxSupportThreshold->blockSignals(oldState);

    // Support texture
    oldState = spinBoxSupportTexture->blockSignals(true);
    spinBoxSupportTexture->setValue(method->getSupportTexture());
    spinBoxSupportTexture->blockSignals(oldState);

    // Candidate step size
    oldState = spinBoxCandidateStepSize->blockSignals(true);
    spinBoxCandidateStepSize->setValue(method->getCandidateStepSize());
    spinBoxCandidateStepSize->blockSignals(oldState);

    // Inconsistent window size
    oldState = spinBoxInconsistentWindowSize->blockSignals(true);
    spinBoxInconsistentWindowSize->setValue(method->getInconsistentWindowSize());
    spinBoxInconsistentWindowSize->blockSignals(oldState);

    // Inconsistent threshold
    oldState = spinBoxInconsistentThreshold->blockSignals(true);
    spinBoxInconsistentThreshold->setValue(method->getInconsistentThreshold());
    spinBoxInconsistentThreshold->blockSignals(oldState);

    // Inconsistent min. support
    oldState = spinBoxInconsistentMinSupport->blockSignals(true);
    spinBoxInconsistentMinSupport->setValue(method->getInconsistentMinSupport());
    spinBoxInconsistentMinSupport->blockSignals(oldState);


    // Add corners
    oldState = checkBoxAddCorners->blockSignals(true);
    checkBoxAddCorners->setChecked(method->getAddCorners());
    checkBoxAddCorners->blockSignals(oldState);

    // Grid size
    oldState = spinBoxGridSize->blockSignals(true);
    spinBoxGridSize->setValue(method->getGridSize());
    spinBoxGridSize->blockSignals(oldState);
    

    // Beta
    oldState = spinBoxBeta->blockSignals(true);
    spinBoxBeta->setValue(method->getBeta());
    spinBoxBeta->blockSignals(oldState);

    // Gamma
    oldState = spinBoxGamma->blockSignals(true);
    spinBoxGamma->setValue(method->getGamma());
    spinBoxGamma->blockSignals(oldState);

    // Sigma
    oldState = spinBoxSigma->blockSignals(true);
    spinBoxSigma->setValue(method->getSigma());
    spinBoxSigma->blockSignals(oldState);

    // Sigma radius
    oldState = spinBoxSigmaRadius->blockSignals(true);
    spinBoxSigmaRadius->setValue(method->getSigmaRadius());
    spinBoxSigmaRadius->blockSignals(oldState);


    // Match texture
    oldState = spinBoxMatchTexture->blockSignals(true);
    spinBoxMatchTexture->setValue(method->getMatchTexture());
    spinBoxMatchTexture->blockSignals(oldState);

    // LR threshold
    oldState = spinBoxLRThreshold->blockSignals(true);
    spinBoxLRThreshold->setValue(method->getLRThreshold());
    spinBoxLRThreshold->blockSignals(oldState);


    // Speckles sim threshold
    oldState = spinBoxSpeckleSimThreshold->blockSignals(true);
    spinBoxSpeckleSimThreshold->setValue(method->getSpeckleSimThreshold());
    spinBoxSpeckleSimThreshold->blockSignals(oldState);

    // Speckle size
    oldState = spinBoxSpeckleSize->blockSignals(true);
    spinBoxSpeckleSize->setValue(method->getSpeckleSize());
    spinBoxSpeckleSize->blockSignals(oldState);

    // Interpolation gap width
    oldState = spinBoxInterpolationGapWidth->blockSignals(true);
    spinBoxInterpolationGapWidth->setValue(method->getInterpolationGapWidth());
    spinBoxInterpolationGapWidth->blockSignals(oldState);


    // Filter median
    oldState = checkBoxFilterMedian->blockSignals(true);
    checkBoxFilterMedian->setChecked(method->getFilterMedian());
    checkBoxFilterMedian->blockSignals(oldState);

    // Filter adaptive mean
    oldState = checkBoxFilterAdaptiveMean->blockSignals(true);
    checkBoxFilterAdaptiveMean->setChecked(method->getFilterAdaptiveMean());
    checkBoxFilterAdaptiveMean->blockSignals(oldState);

    // Post-process only left
    oldState = checkBoxPostProcessOnlyLeft->blockSignals(true);
    checkBoxPostProcessOnlyLeft->setChecked(method->getPostProcessOnlyLeft());
    checkBoxPostProcessOnlyLeft->blockSignals(oldState);

    // Subsampling
    oldState = checkBoxSubsampling->blockSignals(true);
    checkBoxSubsampling->setChecked(method->getSubsampling());
    checkBoxSubsampling->blockSignals(oldState);


    // Return left
    oldState = checkBoxReturnLeft->blockSignals(true);
    checkBoxReturnLeft->setChecked(method->getReturnLeft());
    checkBoxReturnLeft->blockSignals(oldState);
}
