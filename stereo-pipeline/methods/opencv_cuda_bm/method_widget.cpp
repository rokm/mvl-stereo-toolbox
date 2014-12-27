/*
 * OpenCV CUDA Block Matching: method widget
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

#include "method_widget.h"
#include "method.h"

#include <cfloat>
#include <climits>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvCudaBm {


MethodWidget::MethodWidget (Method *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    connect(method, &Method::parameterChanged, this, &MethodWidget::updateParameters);

    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QPushButton *button;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QDoubleSpinBox *spinBoxD;
    QFrame *line;

    QString tooltip;

    // Name
    label = new QLabel("<b><u>OpenCV CUDA block matching</u></b>", this);
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


    // Defaults
    tooltip = "Reset to default OpenCV values.";

    button = new QPushButton("Default values");
    button->setToolTip(tooltip);
    connect(button, &QPushButton::clicked, method, &Method::resetToDefaults);

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Pre-filter
    tooltip = "Pre-filter setting.";

    label = new QLabel("Pre-filter", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("NONE", cv::StereoBM::PREFILTER_NORMALIZED_RESPONSE);
    comboBox->setItemData(0, "Basic mode without pre-filtering.", Qt::ToolTipRole);
    comboBox->addItem("XSOBEL", cv::StereoBM::PREFILTER_XSOBEL);
    comboBox->setItemData(1, "Sobel pre-filtering mode.", Qt::ToolTipRole);

    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [this, comboBox] (int index) {
        method->setPreFilterType(comboBox->itemData(index).toInt());
    });

    comboBoxPreFilterType = comboBox;

    layout->addRow(label, comboBox);

    // Pre-filter cap
    tooltip = "Truncation value for the pre-filtered image pixels; up to ~31.";

    label = new QLabel("Pre-filter cap", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 63);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setPreFilterCap);
    spinBoxPreFilterCap = spinBox;

    layout->addRow(label, spinBox);

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
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setNumDisparities);
    spinBoxNumDisparities = spinBox;

    layout->addRow(label, spinBox);

    // Window size
    tooltip = "Block size.";

    label = new QLabel("Window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(3, 63);
    spinBox->setSingleStep(2); // Must not be divisible by 2
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setWindowSize);
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
    spinBoxD->setRange(0.0, 9999.0);
    connect(spinBoxD, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), method, &Method::setAverageTextureThreshold);
    spinBoxAverageTextureThreshold = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Update parameters
    updateParameters();
}

MethodWidget::~MethodWidget ()
{
}


void MethodWidget::updateParameters ()
{
    bool oldState;

    // Pre-filter type
    oldState = comboBoxPreFilterType->blockSignals(true);
    comboBoxPreFilterType->setCurrentIndex(comboBoxPreFilterType->findData(method->getPreFilterType()));
    comboBoxPreFilterType->blockSignals(oldState);

    // Pre-filter cap
    oldState = spinBoxPreFilterCap->blockSignals(true);
    spinBoxPreFilterCap->setValue(method->getPreFilterCap());
    spinBoxPreFilterCap->blockSignals(oldState);


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


} // StereoMethodOpenCvCudaBm
} // Pipeline
} // StereoToolbox
} // MVL
