/*
 * OpenCV GPU Belief Propagation: config widget
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

#include <cfloat>
#include <climits>

using namespace StereoMethodBeliefPropagationGPU;


MethodWidget::MethodWidget (Method *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QDoubleSpinBox *spinBoxD;
    QFrame *line;

    QString tooltip;

    // Name
    label = new QLabel("<b><u>OpenCV GPU belief propagation</u></b>", this);
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
    comboBox->addItem("OpenCV - init", Method::OpenCVInit);
    comboBox->setItemData(0, "Initial OpenCV settings.", Qt::ToolTipRole);
    comboBox->addItem("OpenCV - recommended", Method::OpenCVRecommended);
    comboBox->setItemData(1, "Recommended parameters estimated from image dimensions.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(presetChanged(int)));
    comboBoxPreset = comboBox;

    layout->addRow(label, comboBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Number of disparities
    tooltip = "Number of disparities.";

    label = new QLabel("Num. disparities", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumDisparities(int)));
    spinBoxNumDisparities = spinBox;

    layout->addRow(label, spinBox);

    // Number of iterations
    tooltip = "Number of BP iterations on each level.";

    label = new QLabel("Iterations", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setIterations(int)));
    spinBoxIterations = spinBox;

    layout->addRow(label, spinBox);

    // Number of levels
    tooltip = "Number of levels.";

    label = new QLabel("Levels", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setLevels(int)));
    spinBoxLevels = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Max data term
    tooltip = "Threshold for data cost truncation.";

    label = new QLabel("Max. data term", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setMaxDataTerm(double)));
    spinBoxMaxDataTerm = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Data weight
    tooltip = "Data weight.";

    label = new QLabel("Data weight", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setDataWeight(double)));
    spinBoxDataWeight = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Max discontinuity term
    tooltip = "Threshold for discontinuity truncation.";

    label = new QLabel("Max. disc. term", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setMaxDiscTerm(double)));
    spinBoxMaxDiscTerm = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Discontinuity single jump
    tooltip = "Discontinuity single jump.";

    label = new QLabel("Disc. single jump", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setDiscSingleJump(double)));
    spinBoxDiscSingleJump = spinBoxD;

    layout->addRow(label, spinBoxD);

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


void MethodWidget::updateParameters ()
{
    bool oldState;

    // Num. disparities
    oldState = spinBoxNumDisparities->blockSignals(true);
    spinBoxNumDisparities->setValue(method->getNumDisparities());
    spinBoxNumDisparities->blockSignals(oldState);


    // Iterations
    oldState = spinBoxIterations->blockSignals(true);
    spinBoxIterations->setValue(method->getIterations());
    spinBoxIterations->blockSignals(oldState);

    // Levels
    oldState = spinBoxLevels->blockSignals(true);
    spinBoxLevels->setValue(method->getLevels());
    spinBoxLevels->blockSignals(oldState);


    // Max. data term
    oldState = spinBoxMaxDataTerm->blockSignals(true);
    spinBoxMaxDataTerm->setValue(method->getMaxDataTerm());
    spinBoxMaxDataTerm->blockSignals(oldState);

    // Data weight
    oldState = spinBoxDataWeight->blockSignals(true);
    spinBoxDataWeight->setValue(method->getDataWeight());
    spinBoxDataWeight->blockSignals(oldState);

    // Max. disc. term
    oldState = spinBoxMaxDiscTerm->blockSignals(true);
    spinBoxMaxDiscTerm->setValue(method->getMaxDiscTerm());
    spinBoxMaxDiscTerm->blockSignals(oldState);

    // Disc. single jump
    oldState = spinBoxDiscSingleJump->blockSignals(true);
    spinBoxDiscSingleJump->setValue(method->getDiscSingleJump());
    spinBoxDiscSingleJump->blockSignals(oldState);
}
