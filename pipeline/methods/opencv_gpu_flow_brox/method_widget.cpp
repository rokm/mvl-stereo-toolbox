/*
 * OpenCV GPU Dense Optical Flow - Brox: config widget
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

using namespace StereoMethodFlowBroxGpu;


MethodWidget::MethodWidget (Method *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QSpinBox *spinBox;
    QDoubleSpinBox *spinBoxD;
    QCheckBox *checkBox;
    QFrame *line;

    QString tooltip;

    // Name
    label = new QLabel("<b><u>OpenCV GPU Flow - Brox</u></b>", this);
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

    // Alpha
    tooltip = "Flow smoothness";

    label = new QLabel("Alpha", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0, 1.0);
    spinBoxD->setDecimals(4);
    spinBoxD->setSingleStep(0.1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setAlpha(double)));
    spinBoxAlpha = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Gamma
    tooltip = "Gradient constancy importance";

    label = new QLabel("Gamma", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0, 1.0);
    spinBoxD->setDecimals(4);
    spinBoxD->setSingleStep(0.1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setGamma(double)));
    spinBoxGamma = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Scale factor
    tooltip = "Pyramid scale factor";

    label = new QLabel("Scale factor", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0, 10);
    spinBoxD->setDecimals(4);
    spinBoxD->setSingleStep(0.1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setScaleFactor(double)));
    spinBoxScaleFactor = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Inner iterations
    tooltip = "Number of lagged non-linearity iterations (inner loop)";

    label = new QLabel("Inner iterations", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 9999);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setInnerIterations(int)));
    spinBoxInnerIterations = spinBox;

    layout->addRow(label, spinBox);

    // Outer iterations
    tooltip = "Number of warping iterations (number of pyramid levels)";

    label = new QLabel("Outer iterations", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 9999);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setOuterIterations(int)));
    spinBoxOuterIterations = spinBox;

    layout->addRow(label, spinBox);

    // Solver iterations
    tooltip = "Number of linear system solver iterations";

    label = new QLabel("Solver iterations", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 9999);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSolverIterations(int)));
    spinBoxSolverIterations = spinBox;

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

    // Alpha
    oldState = spinBoxAlpha->blockSignals(true);
    spinBoxAlpha->setValue(method->getAlpha());
    spinBoxAlpha->blockSignals(oldState);

    // Gamma
    oldState = spinBoxGamma->blockSignals(true);
    spinBoxGamma->setValue(method->getGamma());
    spinBoxGamma->blockSignals(oldState);

    // Scale factor
    oldState = spinBoxScaleFactor->blockSignals(true);
    spinBoxScaleFactor->setValue(method->getScaleFactor());
    spinBoxScaleFactor->blockSignals(oldState);

    // Inner iterations
    oldState = spinBoxInnerIterations->blockSignals(true);
    spinBoxInnerIterations->setValue(method->getInnerIterations());
    spinBoxInnerIterations->blockSignals(oldState);

    // Outer iterations
    oldState = spinBoxOuterIterations->blockSignals(true);
    spinBoxOuterIterations->setValue(method->getOuterIterations());
    spinBoxOuterIterations->blockSignals(oldState);

    // Solver iterations
    oldState = spinBoxSolverIterations->blockSignals(true);
    spinBoxSolverIterations->setValue(method->getSolverIterations());
    spinBoxSolverIterations->blockSignals(oldState);
}
