/*
 * OpenCV GPU Dense Optical Flow - Farneback: config widget
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

using namespace StereoMethodFlowFarnebackGpu;


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
    label = new QLabel("<b><u>OpenCV GPU Flow - Farneback</u></b>", this);
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

    // Number of levels
    tooltip = "Number of pyramid layers including the initial image (1 means that only original images are used).";

    label = new QLabel("Number of levels", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 9999);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumberOfLevels(int)));
    spinBoxNumberOfLevels = spinBox;

    layout->addRow(label, spinBox);

    // Pyramid scale
    tooltip = "Image scale to build pyramids for each image; 0.5 means a classical pyramid where\n"
              "each next layer is twice smaller than the previous one.";

    label = new QLabel("Pyramid scale", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0, 1.0);
    spinBoxD->setDecimals(4);
    spinBoxD->setSingleStep(0.1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setPyramidScale(double)));
    spinBoxPyramidScale = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Reverse images
    tooltip = "Fast pyramids.";

    checkBox = new QCheckBox("Fast pyramids", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(toggled(bool)), method, SLOT(setFastPyramids(bool)));
    checkBoxFastPyramids= checkBox;

    layout->addRow(checkBox);

    // Window size
    tooltip = "Averaging window size; larger values increase the algorithm robustness to image noise and\n"
              "give more chances for fast motion detection, but yield more blurred motion field.";

    label = new QLabel("Window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 9999);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setWindowSize(int)));
    spinBoxWindowSize = spinBox;

    layout->addRow(label, spinBox);

    // Number of iterations
    tooltip = "Number of iterations at each pyramid level.";

    label = new QLabel("Number of iterations", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 9999);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumberOfIterations(int)));
    spinBoxNumberOfIterations = spinBox;

    layout->addRow(label, spinBox);

    // PolyN
    tooltip = "Size of the pixel neighborhood used to find polynomial expansion in each pixel; larger values\n"
              "mean that the image will be approximated with smoother surfaces, yielding more robust algorithm\n"
              "and more blurred motion field. Typical value: 5 or 7";

    label = new QLabel("Poly N", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 9999);
    spinBox->setSingleStep(1);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPolyN(int)));
    spinBoxPolyN = spinBox;

    layout->addRow(label, spinBox);

    // PolySigma
    tooltip = "Standard deviation of the Gaussian that is used to smooth derivatives used as a basis for the polynomial\n"
              "expansion; for poly_n = 5, you can set poly_sigma = 1.1, for poly_n = 7, a good value would be poly_sigma = 1.5";

    label = new QLabel("Poly Sigma", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0, 9999.0);
    spinBoxD->setDecimals(4);
    spinBoxD->setSingleStep(1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setPolySigma(double)));
    spinBoxPolySigma = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Use Gaussian filter
    tooltip = "Use the Gaussian filter instead of a box filter of the same size for optical flow estimation; usually, this\n"
              "option gives z more accurate flow than with a box filter, at the cost of lower speed; normally, window size\n"
              "for a Gaussian window should be set to a larger value to achieve the same level of robustness.";

    checkBox = new QCheckBox("Gaussian filter", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(toggled(bool)), method, SLOT(setUseGaussianFilter(bool)));
    checkBoxUseGaussianFilter = checkBox;

    layout->addRow(checkBox);

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

    // Number of levels
    oldState = spinBoxNumberOfLevels->blockSignals(true);
    spinBoxNumberOfLevels->setValue(method->getNumberOfLevels());
    spinBoxNumberOfLevels->blockSignals(oldState);

    // Pyramid scale
    oldState = spinBoxPyramidScale->blockSignals(true);
    spinBoxPyramidScale->setValue(method->getPyramidScale());
    spinBoxPyramidScale->blockSignals(oldState);

    // Fast pyramids
    oldState = checkBoxFastPyramids->blockSignals(true);
    checkBoxFastPyramids->setChecked(method->getFastPyramids());
    checkBoxFastPyramids->blockSignals(oldState);

    // Window size
    oldState = spinBoxWindowSize->blockSignals(true);
    spinBoxWindowSize->setValue(method->getWindowSize());
    spinBoxWindowSize->blockSignals(oldState);

    // Number of iterations
    oldState = spinBoxNumberOfIterations->blockSignals(true);
    spinBoxNumberOfIterations->setValue(method->getNumberOfIterations());
    spinBoxNumberOfIterations->blockSignals(oldState);

    // PolyN
    oldState = spinBoxPolyN->blockSignals(true);
    spinBoxPolyN->setValue(method->getPolyN());
    spinBoxPolyN->blockSignals(oldState);

    // PolySigma
    oldState = spinBoxPolySigma->blockSignals(true);
    spinBoxPolySigma->setValue(method->getPolySigma());
    spinBoxPolySigma->blockSignals(oldState);

    // Use Gaussian filter
    oldState = checkBoxUseGaussianFilter->blockSignals(true);
    checkBoxUseGaussianFilter->setChecked(method->getUseGaussianFilter());
    checkBoxUseGaussianFilter->blockSignals(oldState);
}
