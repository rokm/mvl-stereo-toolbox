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
    connect(method, &Method::parameterChanged, this, &MethodWidget::updateParameters);

    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QDoubleSpinBox *spinBoxD;
    QFrame *line;
    QCheckBox *checkBox;

    QString tooltip;

    // Name
    label = new QLabel("<b><u>Efficient LArge-scale Stereo</u></b>", this);
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
    comboBox->addItem("Robotics", Method::ElasRobotics);
    comboBox->setItemData(0, "Default settings in robotics environment. Does not produce results in \nhalf-occluded areas and is a bit more robust to lighting, etc.", Qt::ToolTipRole);
    comboBox->addItem("Middlebury", Method::ElasMiddlebury);
    comboBox->setItemData(1, "Default settings for Middlebury benchmark. Interpolates all missing disparities.", Qt::ToolTipRole);

    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [this, comboBox] (int index) {
        method->usePreset(comboBox->itemData(index).toInt());
    });

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
    spinBox->setRange(-9999, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setMinDisparity);
    spinBoxMinDisparity = spinBox;

    layout->addRow(label, spinBox);

    // Max disparity
    tooltip = "Maximum possible disparity value.";

    label = new QLabel("Max. disparity", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-9999, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setMaxDisparity);
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
    spinBoxD->setRange(0.0, 9999.0);
    connect(spinBoxD, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), method, &Method::setSupportThreshold);
    spinBoxSupportThreshold = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Support texture
    tooltip = "Min. texture for support points.";

    label = new QLabel("Support texture", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-9999, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setSupportTexture);
    spinBoxSupportTexture = spinBox;

    layout->addRow(label, spinBox);

    // Candidate step size
    tooltip = "Step size of regular grid on which support points are matched.";

    label = new QLabel("Candidate step size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-9999, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setCandidateStepSize);
    spinBoxCandidateStepSize = spinBox;

    layout->addRow(label, spinBox);

    // Inconsistent window size
    tooltip = "Window size of inconsistent support point check";

    label = new QLabel("Incon. window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-9999, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setInconsistentWindowSize);
    spinBoxInconsistentWindowSize = spinBox;

    layout->addRow(label, spinBox);

    // Inconsistent threshold
    tooltip = "Disparity similarity threshold for support point to be considered consistent.";

    label = new QLabel("Incon. threshold", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-9999, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setInconsistentThreshold);
    spinBoxInconsistentThreshold = spinBox;

    layout->addRow(label, spinBox);

    // Inconsistent min support
    tooltip = "Minimum number of consistent support points.";

    label = new QLabel("Incon. min. support", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-9999, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setInconsistentMinSupport);
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
    connect(checkBox, &QCheckBox::toggled, method, &Method::setAddCorners);
    checkBoxAddCorners = checkBox;

    layout->addRow(checkBox);

    // Grid size
    tooltip = "Size of neighborhood for additional support point extrapolation.";

    label = new QLabel("Grid size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setGridSize);
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
    spinBoxD->setRange(0.0, 9999.0);
    connect(spinBoxD, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), method, &Method::setBeta);
    spinBoxBeta = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Gamma
    tooltip = "Prior constant.";

    label = new QLabel("Gamma", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, 9999.0);
    connect(spinBoxD, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), method, &Method::setGamma);
    spinBoxGamma = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Sigma
    tooltip = "Prior sigma.";

    label = new QLabel("Sigma", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, 9999.0);
    connect(spinBoxD, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), method, &Method::setSigma);
    spinBoxSigma = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Sigma radius
    tooltip = "Prior sigma radius.";

    label = new QLabel("Sigma radius", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, 9999.0);
    connect(spinBoxD, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), method, &Method::setSigmaRadius);
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
    spinBox->setRange(0, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setMatchTexture);
    spinBoxMatchTexture = spinBox;

    layout->addRow(label, spinBox);

    // Left/right threshold
    tooltip = "Disparity threshold for left/right consistency check.";

    label = new QLabel("Left/right thr.", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setLRThreshold);
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
    spinBoxD->setRange(0.0, 9999.0);
    connect(spinBoxD, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), method, &Method::setSpeckleSimThreshold);
    spinBoxSpeckleSimThreshold = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Speckle size
    tooltip = "Maximal size of a speckle (small speckles are removed).";

    label = new QLabel("Speckle size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setSpeckleSize);
    spinBoxSpeckleSize = spinBox;

    layout->addRow(label, spinBox);

    // Interpolation gap width
    tooltip = "Interpolate small gaps (left<->right, top<->bottom).";

    label = new QLabel("Interp. gap width", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setInterpolationGapWidth);
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
    connect(checkBox, &QCheckBox::toggled, method, &Method::setFilterMedian);
    checkBoxFilterMedian = checkBox;

    layout->addRow(checkBox);

    // Adaptive mean filter
    tooltip = "Optional adaptive mean filter (approximated).";

    checkBox = new QCheckBox("Adaptive mean filter", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, &QCheckBox::toggled, method, &Method::setFilterAdaptiveMean);
    checkBoxFilterAdaptiveMean = checkBox;

    layout->addRow(checkBox);

    // Postprocess only left
    tooltip = "Save time by not post-processing the right disparity image.";

    checkBox = new QCheckBox("Post-process only left", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, &QCheckBox::toggled, method, &Method::setPostProcessOnlyLeft);
    checkBoxPostProcessOnlyLeft = checkBox;

    layout->addRow(checkBox);

    // Subsampling
    tooltip = "Save time by only computing disparities for each 2nd pixel.";

    checkBox = new QCheckBox("Subsampling", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, &QCheckBox::toggled, method, &Method::setSubsampling);
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
    connect(checkBox, &QCheckBox::toggled, method, &Method::setReturnLeft);
    checkBoxReturnLeft = checkBox;

    layout->addRow(checkBox);

    // Update parameters
    updateParameters();
}

MethodWidget::~MethodWidget ()
{
}

void MethodWidget::updateParameters ()
{
    // Min disparity
    spinBoxMinDisparity->blockSignals(true);
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxMinDisparity->blockSignals(false);

    // Max disparity
    spinBoxMaxDisparity->blockSignals(true);
    spinBoxMaxDisparity->setValue(method->getMaxDisparity());
    spinBoxMaxDisparity->blockSignals(false);


    // Support threshold
    spinBoxSupportThreshold->blockSignals(true);
    spinBoxSupportThreshold->setValue(method->getSupportThreshold());
    spinBoxSupportThreshold->blockSignals(false);

    // Support texture
    spinBoxSupportTexture->blockSignals(true);
    spinBoxSupportTexture->setValue(method->getSupportTexture());
    spinBoxSupportTexture->blockSignals(false);

    // Candidate step size
    spinBoxCandidateStepSize->blockSignals(true);
    spinBoxCandidateStepSize->setValue(method->getCandidateStepSize());
    spinBoxCandidateStepSize->blockSignals(false);

    // Inconsistent window size
    spinBoxInconsistentWindowSize->blockSignals(true);
    spinBoxInconsistentWindowSize->setValue(method->getInconsistentWindowSize());
    spinBoxInconsistentWindowSize->blockSignals(false);

    // Inconsistent threshold
    spinBoxInconsistentThreshold->blockSignals(true);
    spinBoxInconsistentThreshold->setValue(method->getInconsistentThreshold());
    spinBoxInconsistentThreshold->blockSignals(false);

    // Inconsistent min. support
    spinBoxInconsistentMinSupport->blockSignals(true);
    spinBoxInconsistentMinSupport->setValue(method->getInconsistentMinSupport());
    spinBoxInconsistentMinSupport->blockSignals(false);


    // Add corners
    checkBoxAddCorners->blockSignals(true);
    checkBoxAddCorners->setChecked(method->getAddCorners());
    checkBoxAddCorners->blockSignals(false);

    // Grid size
    spinBoxGridSize->blockSignals(true);
    spinBoxGridSize->setValue(method->getGridSize());
    spinBoxGridSize->blockSignals(false);


    // Beta
    spinBoxBeta->blockSignals(true);
    spinBoxBeta->setValue(method->getBeta());
    spinBoxBeta->blockSignals(false);

    // Gamma
    spinBoxGamma->blockSignals(true);
    spinBoxGamma->setValue(method->getGamma());
    spinBoxGamma->blockSignals(false);

    // Sigma
    spinBoxSigma->blockSignals(true);
    spinBoxSigma->setValue(method->getSigma());
    spinBoxSigma->blockSignals(false);

    // Sigma radius
    spinBoxSigmaRadius->blockSignals(true);
    spinBoxSigmaRadius->setValue(method->getSigmaRadius());
    spinBoxSigmaRadius->blockSignals(false);


    // Match texture
    spinBoxMatchTexture->blockSignals(true);
    spinBoxMatchTexture->setValue(method->getMatchTexture());
    spinBoxMatchTexture->blockSignals(false);

    // LR threshold
    spinBoxLRThreshold->blockSignals(true);
    spinBoxLRThreshold->setValue(method->getLRThreshold());
    spinBoxLRThreshold->blockSignals(false);


    // Speckles sim threshold
    spinBoxSpeckleSimThreshold->blockSignals(true);
    spinBoxSpeckleSimThreshold->setValue(method->getSpeckleSimThreshold());
    spinBoxSpeckleSimThreshold->blockSignals(false);

    // Speckle size
    spinBoxSpeckleSize->blockSignals(true);
    spinBoxSpeckleSize->setValue(method->getSpeckleSize());
    spinBoxSpeckleSize->blockSignals(false);

    // Interpolation gap width
    spinBoxInterpolationGapWidth->blockSignals(true);
    spinBoxInterpolationGapWidth->setValue(method->getInterpolationGapWidth());
    spinBoxInterpolationGapWidth->blockSignals(false);


    // Filter median
    checkBoxFilterMedian->blockSignals(true);
    checkBoxFilterMedian->setChecked(method->getFilterMedian());
    checkBoxFilterMedian->blockSignals(false);

    // Filter adaptive mean
    checkBoxFilterAdaptiveMean->blockSignals(true);
    checkBoxFilterAdaptiveMean->setChecked(method->getFilterAdaptiveMean());
    checkBoxFilterAdaptiveMean->blockSignals(false);

    // Post-process only left
    checkBoxPostProcessOnlyLeft->blockSignals(true);
    checkBoxPostProcessOnlyLeft->setChecked(method->getPostProcessOnlyLeft());
    checkBoxPostProcessOnlyLeft->blockSignals(false);

    // Subsampling
    checkBoxSubsampling->blockSignals(true);
    checkBoxSubsampling->setChecked(method->getSubsampling());
    checkBoxSubsampling->blockSignals(false);


    // Return left
    checkBoxReturnLeft->blockSignals(true);
    checkBoxReturnLeft->setChecked(method->getReturnLeft());
    checkBoxReturnLeft->blockSignals(false);
}
