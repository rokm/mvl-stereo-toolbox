/*
 * OpenCV GPU Block Matching: config widget
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
 
#include "StereoMethodBlockMatchingGPUConfigWidget.h"
#include "StereoMethodBlockMatchingGPU.h"

#include <cfloat>
#include <climits>


StereoMethodBlockMatchingGPUConfigWidget::StereoMethodBlockMatchingGPUConfigWidget (StereoMethodBlockMatchingGPU *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QPushButton *button;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QDoubleSpinBox *spinBoxD;
    QFrame *line;

    QString tooltip;

    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Name
    label = new QLabel("<b><u>OpenCV GPU block matching</u></b>", this);
    label->setAlignment(Qt::AlignHCenter);

    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Defaults
    tooltip = "Reset to default OpenCV values.";

    button = new QPushButton("Default values");
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), method, SLOT(resetToDefaults()));

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Preset
    tooltip = "Parameter presetting.";
    
    label = new QLabel("Preset", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("BASIC_PRESET", cv::gpu::StereoBM_GPU::BASIC_PRESET);
    comboBox->setItemData(0, "Basic mode without pre-processing.", Qt::ToolTipRole);
    comboBox->addItem("PREFILTER_XSOBEL", cv::gpu::StereoBM_GPU::PREFILTER_XSOBEL);
    comboBox->setItemData(1, "Sobel pre-filtering mode.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(presetChanged(int)));
    comboBoxPreset = comboBox;

    layout->addRow(label, comboBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Num disparities
    tooltip = "Number of disparities. It must be a multiple of 8 and less or equal to 256.";
    
    label = new QLabel("Num. disparities", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(8, 256);
    spinBox->setSingleStep(8); // Must be divisible by 8
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumDisparities(int)));
    spinBoxNumDisparities = spinBox;

    layout->addRow(label, spinBox);

    // Window size
    tooltip = "Block size.";

    label = new QLabel("Window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(2, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setWindowSize(int)));
    spinBoxWindowSize = spinBox;

    layout->addRow(label, spinBox);

    // Average texture threshold
    tooltip = "If avergeTexThreshold != 0 then disparity is set 0 in each point (x,y) where for left image \n"
              "SumOfHorizontalGradiensInWindow(x, y, winSize) < (winSize * winSize) * avergeTexThreshold, \n"
              "i.e. input left image is low textured.";

    label = new QLabel("Avg. texture thr.", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setAverageTextureThreshold(double)));
    spinBoxAverageTextureThreshold = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Update parameters
    updateParameters();
}

StereoMethodBlockMatchingGPUConfigWidget::~StereoMethodBlockMatchingGPUConfigWidget ()
{
}

void StereoMethodBlockMatchingGPUConfigWidget::presetChanged (int index)
{
    method->setPreset(comboBoxPreset->itemData(index).toInt());
}


void StereoMethodBlockMatchingGPUConfigWidget::updateParameters ()
{
    bool oldState;

    // Preset
    oldState = comboBoxPreset->blockSignals(true);
    comboBoxPreset->setCurrentIndex(comboBoxPreset->findData(method->getPreset()));
    comboBoxPreset->blockSignals(oldState);


    // Num. disparities
    oldState = spinBoxNumDisparities->blockSignals(true);
    spinBoxNumDisparities->setValue(method->getNumDisparities());
    spinBoxNumDisparities->blockSignals(oldState);

    // Window size
    oldState = spinBoxWindowSize->blockSignals(true);
    spinBoxWindowSize->setValue(method->getWindowSize());
    spinBoxWindowSize->blockSignals(oldState);

    // Average texture threshold
    oldState = spinBoxAverageTextureThreshold->blockSignals(true);
    spinBoxAverageTextureThreshold->setValue(method->getAverageTextureThreshold());
    spinBoxAverageTextureThreshold->blockSignals(oldState);
}
