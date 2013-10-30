/*
 * OpenCV GPU Dense Optical Flow - Lukas-Kanade with pyramids: config widget
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

using namespace StereoMethodFlowPyrLKGpu;


MethodWidget::MethodWidget (Method *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QSpinBox *spinBox;
    QCheckBox *checkBox;
    QFrame *line;

    QString tooltip;

    // Name
    label = new QLabel("<b><u>OpenCV GPU Flow - PyrLK</u></b>", this);
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
    

    // Reverse images
    tooltip = "Reverse input images.";
    
    checkBox = new QCheckBox("Reverse images", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(toggled(bool)), method, SLOT(setReverseImages(bool)));
    checkBoxReverseImages = checkBox;

    layout->addRow(checkBox);

    // Window width
    tooltip = "Width of search window at each pyramid level.";

    label = new QLabel("Window width", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setWindowWidth(int)));
    spinBoxWindowWidth = spinBox;

    layout->addRow(label, spinBox);

    // Window height
    tooltip = "Height of search window at each pyramid level.";

    label = new QLabel("Window height", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setWindowHeight(int)));
    spinBoxWindowHeight = spinBox;

    layout->addRow(label, spinBox);

    // Max level
    tooltip = "0-based maximal pyramid level number; if set to 0, pyramids are not used (single level).";

    label = new QLabel("Max level", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMaxLevel(int)));
    spinBoxMaxLevel = spinBox;

    layout->addRow(label, spinBox);

    // Number of iterations
    tooltip = "Number of iterations at each pyramid level.";

    label = new QLabel("Number of iterations", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumberOfIterations(int)));
    spinBoxNumberOfIterations = spinBox;

    layout->addRow(label, spinBox);

    // Update parameters
    updateParameters();
}

MethodWidget::~MethodWidget ()
{
}

void MethodWidget::updateParameters ()
{
    bool oldState;

    // Reverse images
    oldState = checkBoxReverseImages->blockSignals(true);
    checkBoxReverseImages->setChecked(method->getReverseImages());
    checkBoxReverseImages->blockSignals(oldState);

    // Window width
    oldState = spinBoxWindowWidth->blockSignals(true);
    spinBoxWindowWidth->setValue(method->getWindowWidth());
    spinBoxWindowWidth->blockSignals(oldState);

    // Window height
    oldState = spinBoxWindowHeight->blockSignals(true);
    spinBoxWindowHeight->setValue(method->getWindowHeight());
    spinBoxWindowHeight->blockSignals(oldState);

    // Max level
    oldState = spinBoxMaxLevel->blockSignals(true);
    spinBoxMaxLevel->setValue(method->getMaxLevel());
    spinBoxMaxLevel->blockSignals(oldState);
    
    // Number of iterations
    oldState = spinBoxNumberOfIterations->blockSignals(true);
    spinBoxNumberOfIterations->setValue(method->getNumberOfIterations());
    spinBoxNumberOfIterations->blockSignals(oldState);
}
