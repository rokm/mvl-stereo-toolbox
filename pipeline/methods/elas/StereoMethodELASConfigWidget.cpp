/*
 * Efficient LArge-scale Stereo: config widget
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

#include "StereoMethodELASConfigWidget.h"
#include "StereoMethodELAS.h"


StereoMethodELASConfigWidget::StereoMethodELASConfigWidget (StereoMethodELAS *m, QWidget *parent)
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
    comboBox->addItem("Robotics", StereoMethodELAS::ElasRobotics);
    comboBox->setItemData(0, "Default settings in robotics environment. Does not produce results in \nhalf-occluded areas and is a bit more robust to lighting, etc.", Qt::ToolTipRole);
    comboBox->addItem("Middlebury", StereoMethodELAS::ElasMiddlebury);
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

StereoMethodELASConfigWidget::~StereoMethodELASConfigWidget ()
{
}

void StereoMethodELASConfigWidget::presetChanged (int index)
{
    method->usePreset(comboBoxPreset->itemData(index).toInt());
}

void StereoMethodELASConfigWidget::addCornersChanged (int state)
{
    method->setAddCorners(state == Qt::Checked);
}

void StereoMethodELASConfigWidget::medianFilterChanged (int state)
{
    method->setFilterMedian(state == Qt::Checked);
}

void StereoMethodELASConfigWidget::adaptiveMeanFilterChanged (int state)
{
    method->setFilterAdaptiveMean(state == Qt::Checked);
}

void StereoMethodELASConfigWidget::postProcessOnlyLeftChanged (int state)
{
    method->setPostProcessOnlyLeft(state == Qt::Checked);
}

void StereoMethodELASConfigWidget::subsamplingChanged (int state)
{
    method->setSubsampling(state == Qt::Checked);
}

void StereoMethodELASConfigWidget::returnLeftChanged (int state)
{
    method->setReturnLeft(state == Qt::Checked);
}

void StereoMethodELASConfigWidget::updateParameters ()
{
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxMaxDisparity->setValue(method->getMaxDisparity());

    spinBoxSupportThreshold->setValue(method->getSupportThreshold());
    spinBoxSupportTexture->setValue(method->getSupportTexture());
    spinBoxCandidateStepSize->setValue(method->getCandidateStepSize());
    spinBoxInconsistentWindowSize->setValue(method->getInconsistentWindowSize());
    spinBoxInconsistentThreshold->setValue(method->getInconsistentThreshold());
    spinBoxInconsistentMinSupport->setValue(method->getInconsistentMinSupport());

    checkBoxAddCorners->setChecked(method->getAddCorners());
    spinBoxGridSize->setValue(method->getGridSize());
    
    spinBoxBeta->setValue(method->getBeta());
    spinBoxGamma->setValue(method->getGamma());
    spinBoxSigma->setValue(method->getSigma());
    spinBoxSigmaRadius->setValue(method->getSigmaRadius());

    spinBoxMatchTexture->setValue(method->getMatchTexture());
    spinBoxLRThreshold->setValue(method->getLRThreshold());

    spinBoxSpeckleSimThreshold->setValue(method->getSpeckleSimThreshold());
    spinBoxSpeckleSize->setValue(method->getSpeckleSize());
    spinBoxInterpolationGapWidth->setValue(method->getInterpolationGapWidth());

    checkBoxFilterMedian->setChecked(method->getFilterMedian());
    checkBoxFilterAdaptiveMean->setChecked(method->getFilterAdaptiveMean());
    checkBoxPostProcessOnlyLeft->setChecked(method->getPostProcessOnlyLeft());
    checkBoxSubsampling->setChecked(method->getSubsampling());

    checkBoxReturnLeft->setChecked(method->getReturnLeft());
}
