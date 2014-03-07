/*
 * OpenCV GPU Dense Optical Flow - Dual TVL1: config widget
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

using namespace StereoMethodFlowDualTVL1Gpu;


MethodWidget::MethodWidget (Method *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QSpinBox *spinBox;
    QCheckBox *checkBox;
    QDoubleSpinBox *spinBoxD;
    QFrame *line;

    QString tooltip;

    // Name
    label = new QLabel("<b><u>OpenCV GPU Flow - Dual TVL1</u></b>", this);
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

    // Tau
    tooltip = "Time step of the numerical scheme.";

    label = new QLabel("Tau", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setDecimals(4);
    spinBoxD->setSingleStep(0.1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setTau(double)));
    spinBoxTau = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Lambda
    tooltip = "Weight parameter for the data term, attachment parameter.\n"
              "This is the most relevant parameter, which determines the smoothness of the output.\n"
              "The smaller this parameter is, the smoother the solutions we obtain.\n"
              "It depends on the range of motions of the images, so its value should be adapted to each image sequence.";

    label = new QLabel("Lambda", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setDecimals(4);
    spinBoxD->setSingleStep(0.1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setLambda(double)));
    spinBoxLambda = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Theta
    tooltip = "Weight parameter for (u - v)^2, tightness parameter.\n"
              "It serves as a link between the attachment and the regularization terms.\n"
              "In theory, it should have a small value in order to maintain both parts in correspondence.\n"
              "The method is stable for a large range of values of this parameter.";

    label = new QLabel("Theta", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setDecimals(4);
    spinBoxD->setSingleStep(0.1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setTheta(double)));
    spinBoxTheta = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Number of scales
    tooltip = "Number of scales used to create the pyramid of images.";

    label = new QLabel("Number of scales", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 9999);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumberOfScales(int)));
    spinBoxNumberOfScales = spinBox;

    layout->addRow(label, spinBox);

    // Number of warpings
    tooltip = "Number of warpings per scale. Represents the number of times\n"
              "that I1(x+u0) and grad( I1(x+u0) ) are computed per scale.\n"
              "This is a parameter that ensures the stability of the method.\n"
              "It also affects the running time, so it is a compromise between speed and accuracy.";

    label = new QLabel("Number of warps", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 9999);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumberOfWarps(int)));
    spinBoxNumberOfWarps = spinBox;

    layout->addRow(label, spinBox);

    // Epsilon
    tooltip = "Stopping criterion threshold used in the numerical scheme, which\n"
              "is a trade-off between precision and running time. A small value\n"
              "will yield more accurate solutions at the expense of a slower convergence.";

    label = new QLabel("Epsilon", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setDecimals(6);
    spinBoxD->setSingleStep(0.1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setEpsilon(double)));
    spinBoxEpsilon = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Number of iterations
    tooltip = "Stopping criterion iterations number used in the numerical scheme.";

    label = new QLabel("Number of iterations", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 9999);
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

    // Tau
    oldState = spinBoxTau->blockSignals(true);
    spinBoxTau->setValue(method->getTau());
    spinBoxTau->blockSignals(oldState);

    // Lambda
    oldState = spinBoxLambda->blockSignals(true);
    spinBoxLambda->setValue(method->getLambda());
    spinBoxLambda->blockSignals(oldState);

    // Theta
    oldState = spinBoxTheta->blockSignals(true);
    spinBoxTheta->setValue(method->getTheta());
    spinBoxTheta->blockSignals(oldState);

    // Number of scales
    oldState = spinBoxNumberOfScales->blockSignals(true);
    spinBoxNumberOfScales->setValue(method->getNumberOfScales());
    spinBoxNumberOfScales->blockSignals(oldState);

    // Number of warps
    oldState = spinBoxNumberOfWarps->blockSignals(true);
    spinBoxNumberOfWarps->setValue(method->getNumberOfWarps());
    spinBoxNumberOfWarps->blockSignals(oldState);

    // Epsilon
    oldState = spinBoxEpsilon->blockSignals(true);
    spinBoxEpsilon->setValue(method->getEpsilon());
    spinBoxEpsilon->blockSignals(oldState);

    // Number of iterations
    oldState = spinBoxNumberOfIterations->blockSignals(true);
    spinBoxNumberOfIterations->setValue(method->getNumberOfIterations());
    spinBoxNumberOfIterations->blockSignals(oldState);
}
