/*
 * OpenCV Variational Matching: config widget
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

using namespace StereoMethodVar;


MethodWidget::MethodWidget (Method *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QComboBox *comboBox;
    QDoubleSpinBox *spinBoxD;
    QSpinBox *spinBox;
    QFrame *line;
    QCheckBox *checkBox;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>OpenCV variational matching</u></b>", this);
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
    comboBox->addItem("OpenCV", Method::OpenCV);
    comboBox->setItemData(0, "Initial OpenCV settings.", Qt::ToolTipRole);
    comboBox->addItem("StereoMatch", Method::StereoMatch);
    comboBox->setItemData(1, "Settings from \"Stereo Match\" example.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(presetChanged(int)));
    comboBoxPreset = comboBox;

    layout->addRow(label, comboBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Levels
    tooltip = "The number of pyramid layers, including the initial image. levels=1 means that no extra layers are \n"
              "created and only the original images are used. This parameter is ignored if flag USE_AUTO_PARAMS is set.";
    
    label = new QLabel("Levels", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setLevels(int)));
    spinBoxLevels = spinBox;

    layout->addRow(label, spinBox);

    // Pyramid scale
    tooltip = "Specifies the image scale (<1) to build the pyramids for each image. pyrScale=0.5 means the classical \n"
              "pyramid, where each next layer is twice smaller than the previous. (This parameter is ignored if flag \n"
              "USE_AUTO_PARAMS is set).";
    
    label = new QLabel("Pyramid scale", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, 1.0);
    spinBoxD->setSingleStep(0.1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setPyrScale(double)));
    spinBoxPyrScale = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Number of iterations
    tooltip = "The number of iterations the algorithm does at each pyramid level. (If the flag USE_SMART_ID is set, the \n"
              "number of iterations will be redistributed in such a way, that more iterations will be done on more \n"
              "coarser levels.)";
    
    label = new QLabel("Number of iterations", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumIterations(int)));
    spinBoxNumIterations = spinBox;

    layout->addRow(label, spinBox);

    // Min disparity
    tooltip = "Minimum possible disparity value. Could be negative in case the left and right input images change places.";
    
    label = new QLabel("Min disparity", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(INT_MIN, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMinDisparity(int)));
    spinBoxMinDisparity = spinBox;

    layout->addRow(label, spinBox);

    // Max disparity
    tooltip = "Maximum possible disparity value.";
    
    label = new QLabel("Max disparity", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(INT_MIN, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMaxDisparity(int)));
    spinBoxMaxDisparity = spinBox;

    layout->addRow(label, spinBox);

    // PolyN
    tooltip = "Size of the pixel neighbourhood used to find polynomial expansion in each pixel. The larger values mean that \n"
              "the image will be approximated with smoother surfaces, yielding more robust algorithm and more blurred motion \n"
              "field. Typically, poly_n = 3, 5 or 7";
    
    label = new QLabel("PolyN", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 10);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPolyN(int)));
    spinBoxPolyN = spinBox;

    layout->addRow(label, spinBox);

    // PolySigma
    tooltip = "Standard deviation of the Gaussian that is used to smooth derivatives that are used as a basis for the polynomial \n"
              "expansion. For poly_n = 5 you can set poly_sigma = 1.1, for poly_n = 7 a good value would be poly_sigma = 1.5.";
              
    label = new QLabel("PolySigma", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, 10.0);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setPolySigma(double)));
    spinBoxPolySigma = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Fi
    tooltip = "The smoothness parameter, or the weight coefficient for the smoothness term.";
    
    label = new QLabel("Fi", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setFi(double)));
    spinBoxFi = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Lambda
    tooltip = "The threshold parameter for edge-preserving smoothness (ignored if PENALIZATION_CHARBONNIER or \n"
              "PENALIZATION_PERONA_MALIK is used).";
    
    label = new QLabel("Lambda", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    spinBoxD->setSingleStep(0.01);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setLambda(double)));
    spinBoxLambda = spinBoxD;

    layout->addRow(label, spinBoxD);

    // Penalization
    tooltip = "Penalization option (ignored if flag USE_AUTO_PARAMS is set).";
    
    label = new QLabel("Penalization", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("TICHONOV", cv::StereoVar::PENALIZATION_TICHONOV);
    comboBox->setItemData(0, "Linear smoothness.", Qt::ToolTipRole);
    comboBox->addItem("CHARBONNIER", cv::StereoVar::PENALIZATION_CHARBONNIER);
    comboBox->setItemData(1, "Non-linear edge preserving smoothness.", Qt::ToolTipRole);
    comboBox->addItem("PERONA_MALIK", cv::StereoVar::PENALIZATION_PERONA_MALIK);
    comboBox->setItemData(2, "Non-linear edge-enhancing smoothness.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(penalizationChanged(int)));
    comboBoxPenalization = comboBox;

    layout->addRow(label, comboBox);

    // Cycle
    tooltip = "Type of the multigrid cycle (ignored if flag USE_AUTO_PARAMS is set).";
    
    label = new QLabel("Cycle", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("Null-cycle", cv::StereoVar::CYCLE_O);
    comboBox->setItemData(0, "Null-cycles.", Qt::ToolTipRole);
    comboBox->addItem("V-cycle", cv::StereoVar::CYCLE_V);
    comboBox->setItemData(1, "V-cycles.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(cycleChanged(int)));
    comboBoxCycle = comboBox;

    layout->addRow(label, comboBox);

    // Flags
    QGroupBox *group = new QGroupBox("Flags", this);
    QVBoxLayout *groupLayout = new QVBoxLayout(group);

    checkBox = new QCheckBox("USE_INITIAL_DISPARITY", this);
    checkBox->setToolTip("Use the input flow as the initial flow approximation.");
    connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(flagsChanged()));
    groupLayout->addWidget(checkBox);
    checkBoxUseInitialDisparity = checkBox;

    checkBox = new QCheckBox("USE_EQUALIZE_HIST", this);
    checkBox->setToolTip("Use the histogram equalization in the pre-processing phase.");
    connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(flagsChanged()));
    groupLayout->addWidget(checkBox);
    checkBoxUseEqualizeHist = checkBox;

    checkBox = new QCheckBox("USE_SMART_ID", this);
    checkBox->setToolTip("Use the smart iteration distribution (SID).");
    connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(flagsChanged()));
    groupLayout->addWidget(checkBox);
    checkBoxUseSmartId = checkBox;

    checkBox = new QCheckBox("USE_AUTO_PARAMS", this);
    checkBox->setToolTip("Allow the method to initialize the main parameters.");
    connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(flagsChanged()));
    groupLayout->addWidget(checkBox);
    checkBoxUseAutoParams = checkBox;

    checkBox = new QCheckBox("USE_MEDIAN_FILTERING", this);
    checkBox->setToolTip("Use the median filer of the solution in the post-processing phase.");
    connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(flagsChanged()));
    groupLayout->addWidget(checkBox);
    checkBoxUseMedianFiltering = checkBox;

    layout->addRow(group);

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

void MethodWidget::penalizationChanged (int index)
{
    method->setPenalization(comboBoxPenalization->itemData(index).toInt());
}

void MethodWidget::cycleChanged (int index)
{
    method->setCycle(comboBoxCycle->itemData(index).toInt());
}

void MethodWidget::flagsChanged ()
{
    int flags = 0;
    flags += checkBoxUseInitialDisparity->isChecked() * cv::StereoVar::USE_INITIAL_DISPARITY;
    flags += checkBoxUseEqualizeHist->isChecked() * cv::StereoVar::USE_EQUALIZE_HIST;
    flags += checkBoxUseSmartId->isChecked() * cv::StereoVar::USE_SMART_ID;
    flags += checkBoxUseAutoParams->isChecked() * cv::StereoVar::USE_AUTO_PARAMS;
    flags += checkBoxUseMedianFiltering->isChecked() * cv::StereoVar::USE_MEDIAN_FILTERING;

    method->setFlags(flags);
}


void MethodWidget::updateParameters ()
{
    bool oldState;

    // Levels
    oldState = spinBoxLevels->blockSignals(true);
    spinBoxLevels->setValue(method->getLevels());
    spinBoxLevels->blockSignals(oldState);

    // Pyr. scale
    oldState = spinBoxPyrScale->blockSignals(true);
    spinBoxPyrScale->setValue(method->getPyrScale());
    spinBoxPyrScale->blockSignals(oldState);

    // Num. iterations
    oldState = spinBoxNumIterations->blockSignals(true);
    spinBoxNumIterations->setValue(method->getNumIterations());
    spinBoxNumIterations->blockSignals(oldState);


    // Min. disparity
    oldState = spinBoxMinDisparity->blockSignals(true);
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxMinDisparity->blockSignals(oldState);

    // Max. disparity
    oldState = spinBoxMaxDisparity->blockSignals(true);
    spinBoxMaxDisparity->setValue(method->getMaxDisparity());
    spinBoxMaxDisparity->blockSignals(oldState);


    // Poly N
    oldState = spinBoxPolyN->blockSignals(true);
    spinBoxPolyN->setValue(method->getPolyN());
    spinBoxPolyN->blockSignals(oldState);

    // Poly Sigma
    oldState = spinBoxPolySigma->blockSignals(true);
    spinBoxPolySigma->setValue(method->getPolySigma());
    spinBoxPolySigma->blockSignals(oldState);


    // Fi
    oldState = spinBoxFi->blockSignals(true);
    spinBoxFi->setValue(method->getFi());
    spinBoxFi->blockSignals(oldState);

    // Lambda
    oldState = spinBoxLambda->blockSignals(true);
    spinBoxLambda->setValue(method->getLambda());
    spinBoxLambda->blockSignals(oldState);


    // Penalization
    oldState = comboBoxPenalization->blockSignals(true);
    comboBoxPenalization->setCurrentIndex(comboBoxPenalization->findData(method->getPenalization()));
    comboBoxPenalization->blockSignals(oldState);

    // Cycle
    oldState = comboBoxCycle->blockSignals(true);
    comboBoxCycle->setCurrentIndex(comboBoxCycle->findData(method->getCycle()));
    comboBoxCycle->blockSignals(oldState);

    // Flags
    int flags = method->getFlags();

    oldState = checkBoxUseInitialDisparity->blockSignals(true);
    checkBoxUseInitialDisparity->setChecked(flags & cv::StereoVar::USE_INITIAL_DISPARITY);
    checkBoxUseInitialDisparity->blockSignals(oldState);

    oldState = checkBoxUseEqualizeHist->blockSignals(true);
    checkBoxUseEqualizeHist->setChecked(flags & cv::StereoVar::USE_EQUALIZE_HIST);
    checkBoxUseEqualizeHist->blockSignals(oldState);

    oldState = checkBoxUseSmartId->blockSignals(true);
    checkBoxUseSmartId->setChecked(flags & cv::StereoVar::USE_SMART_ID);
    checkBoxUseSmartId->blockSignals(oldState);

    oldState = checkBoxUseAutoParams->blockSignals(true);
    checkBoxUseAutoParams->setChecked(flags & cv::StereoVar::USE_AUTO_PARAMS);
    checkBoxUseAutoParams->blockSignals(oldState);

    oldState = checkBoxUseMedianFiltering->blockSignals(true);
    checkBoxUseMedianFiltering->setChecked(flags & cv::StereoVar::USE_MEDIAN_FILTERING);
    checkBoxUseMedianFiltering->blockSignals(oldState);
}
