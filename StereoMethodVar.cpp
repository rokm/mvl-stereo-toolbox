/*
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
 
#include "StereoMethodVar.h"

#include <cfloat>
#include <climits>
#include <opencv2/imgproc/imgproc.hpp>


StereoMethodVar::StereoMethodVar (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "Var";
    configWidget = new ConfigTabVar(this);

    usePreset(OpenCV);
}

StereoMethodVar::~StereoMethodVar ()
{
    //delete configWidget;
}

void StereoMethodVar::usePreset (int type)
{
    switch (type) {
        case OpenCV: {
            // OpenCV
            var = cv::StereoVar();
            //var.numberOfDisparities = ((imageWidth/8) + 15) & -16;
            break;
        }
        case StereoMatch: {
            // "Stereo match" example
            var = cv::StereoVar();

            var.levels = 3;
            var.pyrScale = 0.5;
            var.nIt = 25;
            var.minDisp = -(((imageWidth/8) + 15) & -16);
            var.maxDisp = 0;
            var.poly_n = 3;
            var.poly_sigma = 0.0;
            var.fi = 15.0f;
            var.lambda = 0.03f;
            var.penalization = var.PENALIZATION_TICHONOV;
            var.cycle = var.CYCLE_V;
            var.flags = var.USE_SMART_ID | var.USE_AUTO_PARAMS | var.USE_INITIAL_DISPARITY | var.USE_MEDIAN_FILTERING;

            break;
        }
    };

    emit parameterChanged();
}



// *********************************************************************
// *                      Depth image computation                      *
// *********************************************************************
void StereoMethodVar::computeDepthImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &depth)
{    
    // Compute depth image
    var(img1, img2, tmpDepth);

    // Normalize to output
    tmpDepth.convertTo(depth, CV_8U);
}


// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethodVar::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
    // Load parameters
    var = cv::StereoVar();
    
    storage["Levels"] >> var.levels;
    storage["PyrScale"] >> var.pyrScale;
    storage["NumIterations"] >> var.nIt;
    storage["MinDisparity"] >> var.minDisp;
    storage["MaxDisparity"] >> var.maxDisp;
    storage["PolyN"] >> var.poly_n;
    storage["PolySigma"] >> var.poly_sigma;
    storage["Fi"] >> var.fi;
    storage["Lambda"] >> var.lambda;
    storage["Penalization"] >> var.penalization;
    storage["Cycle"] >> var.cycle;
    storage["Flags"] >> var.flags;
    
    emit parameterChanged();
}

void StereoMethodVar::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);

    // Save parameters
    storage << "Levels" << var.levels;
    storage << "PyrScale" << var.pyrScale;
    storage << "NumIterations" << var.nIt;
    storage << "MinDisparity" << var.minDisp;
    storage << "MaxDisparity" << var.maxDisp;
    storage << "PolyN" << var.poly_n;
    storage << "PolySigma" << var.poly_sigma;
    storage << "Fi" << var.fi;
    storage << "Lambda" << var.lambda;
    storage << "Penalization" << var.penalization;
    storage << "Cycle" << var.cycle;
    storage << "Flags" << var.flags;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Levels
int StereoMethodVar::getLevels () const
{
    return var.levels;
}

void StereoMethodVar::setLevels (int newValue)
{
    setParameter(var.levels, newValue);
}

// Pyramid scale
double StereoMethodVar::getPyrScale () const
{
    return var.pyrScale;
}

void StereoMethodVar::setPyrScale (double newValue)
{
    setParameter(var.pyrScale, newValue);
}
    
// Number of iterations
int StereoMethodVar::getNumIterations () const
{
    return var.nIt;
}

void StereoMethodVar::setNumIterations (int newValue)
{
    setParameter(var.nIt, newValue);
}
        

// Minimum disparity
int StereoMethodVar::getMinDisparity () const
{
    return var.minDisp;
}

void StereoMethodVar::setMinDisparity (int newValue)
{
    setParameter(var.minDisp, newValue);
}
    
// Maximum disparity
int StereoMethodVar::getMaxDisparity () const
{
    return var.maxDisp;
}

void StereoMethodVar::setMaxDisparity (int newValue)
{
    setParameter(var.maxDisp, newValue);
}

// PolyN
int StereoMethodVar::getPolyN () const
{
    return var.poly_n;
}

void StereoMethodVar::setPolyN (int newValue)
{
    setParameter(var.poly_n, newValue);
}

// PolySigma
double StereoMethodVar::getPolySigma () const
{
    return var.poly_sigma;
}

void StereoMethodVar::setPolySigma (double newValue)
{
    setParameter(var.poly_sigma, newValue);
}

// Fi
double StereoMethodVar::getFi () const
{
    return var.fi;
}

void StereoMethodVar::setFi (double newValue)
{
    setParameter(var.fi, (float)newValue);
}

// Lambda
double StereoMethodVar::getLambda () const
{
    return var.lambda;
}

void StereoMethodVar::setLambda (double newValue)
{
    setParameter(var.lambda, (float)newValue);
}
 
// Penalization
int StereoMethodVar::getPenalization () const
{
    return var.penalization;
}

void StereoMethodVar::setPenalization (int newValue)
{
    if (newValue != cv::StereoVar::PENALIZATION_TICHONOV &&
        newValue != cv::StereoVar::PENALIZATION_CHARBONNIER &&
        newValue != cv::StereoVar::PENALIZATION_PERONA_MALIK) {
        newValue = cv::StereoVar::PENALIZATION_TICHONOV;
    }

    setParameter(var.penalization, newValue);
}

// Cycle
int StereoMethodVar::getCycle () const
{
    return var.cycle;
}

void StereoMethodVar::setCycle (int newValue)
{
    if (newValue != cv::StereoVar::CYCLE_O &&
        newValue != cv::StereoVar::CYCLE_V) {
        newValue = cv::StereoVar::CYCLE_O;
    }

    setParameter(var.cycle, newValue);
}

// Flags
int StereoMethodVar::getFlags () const
{
    return var.flags;
}

void StereoMethodVar::setFlags (int newValue)
{
    setParameter(var.flags, newValue);
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabVar::ConfigTabVar (StereoMethodVar *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    QLabel *label;
    QComboBox *comboBox;
    QDoubleSpinBox *spinBoxD;
    QSpinBox *spinBox;
    QFrame *line;
    QCheckBox *checkBox;
    QString tooltip;

    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Name
    label = new QLabel("<b><u>OpenCV variational matching</u></b>", this);
    label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(label, row, 0, 1, 2);

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Preset
    tooltip = "Presets for quick initialization.";
    
    label = new QLabel("Preset", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    comboBox = new QComboBox(this);
    comboBox->addItem("OpenCV", StereoMethodVar::OpenCV);
    comboBox->setItemData(0, "Initial OpenCV settings.", Qt::ToolTipRole);
    comboBox->addItem("StereoMatch", StereoMethodVar::StereoMatch);
    comboBox->setItemData(1, "Settings from \"Stereo Match\" example.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(presetChanged(int)));
    layout->addWidget(comboBox, row, 1);
    comboBoxPreset = comboBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Levels
    tooltip = "The number of pyramid layers, including the initial image. levels=1 means that no extra layers are \n"
              "created and only the original images are used. This parameter is ignored if flag USE_AUTO_PARAMS is set.";
    
    label = new QLabel("Levels", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setLevels(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxLevels = spinBox;

    row++;

    // Pyramid scale
    tooltip = "Specifies the image scale (<1) to build the pyramids for each image. pyrScale=0.5 means the classical \n"
              "pyramid, where each next layer is twice smaller than the previous. (This parameter is ignored if flag \n"
              "USE_AUTO_PARAMS is set).";
    
    label = new QLabel("Pyramid scale", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, 1.0);
    spinBoxD->setSingleStep(0.1);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setPyrScale(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxPyrScale = spinBoxD;

    row++;

    // Number of iterations
    tooltip = "The number of iterations the algorithm does at each pyramid level. (If the flag USE_SMART_ID is set, the \n"
              "number of iterations will be redistributed in such a way, that more iterations will be done on more \n"
              "coarser levels.)";
    
    label = new QLabel("Number of iterations", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumIterations(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxNumIterations = spinBox;

    row++;

    // Min disparity
    tooltip = "Minimum possible disparity value. Could be negative in case the left and right input images change places.";
    
    label = new QLabel("Min disparity", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(INT_MIN, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMinDisparity(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxMinDisparity = spinBox;

    row++;

    // Max disparity
    tooltip = "Maximum possible disparity value.";
    
    label = new QLabel("Max disparity", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(INT_MIN, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMaxDisparity(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxMaxDisparity = spinBox;

    row++;

    // PolyN
    tooltip = "Size of the pixel neighbourhood used to find polynomial expansion in each pixel. The larger values mean that \n"
              "the image will be approximated with smoother surfaces, yielding more robust algorithm and more blurred motion \n"
              "field. Typically, poly_n = 3, 5 or 7";
    
    label = new QLabel("PolyN", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 10);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPolyN(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxPolyN = spinBox;

    row++;

    // PolySigma
    tooltip = "Standard deviation of the Gaussian that is used to smooth derivatives that are used as a basis for the polynomial \n"
              "expansion. For poly_n = 5 you can set poly_sigma = 1.1, for poly_n = 7 a good value would be poly_sigma = 1.5.";
              
    label = new QLabel("PolySigma", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, 10.0);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setPolySigma(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxPolySigma = spinBoxD;

    row++;

    // Fi
    tooltip = "The smoothness parameter, or the weight coefficient for the smoothness term.";
    
    label = new QLabel("Fi", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setFi(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxFi = spinBoxD;

    row++;

    // Lambda
    tooltip = "The threshold parameter for edge-preserving smoothness (ignored if PENALIZATION_CHARBONNIER or \n"
              "PENALIZATION_PERONA_MALIK is used).";
    
    label = new QLabel("Lambda", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    spinBoxD->setSingleStep(0.01);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setLambda(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxLambda = spinBoxD;

    row++;

    // Penalization
    tooltip = "Penalization option (ignored if flag USE_AUTO_PARAMS is set).";
    
    label = new QLabel("Penalization", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    comboBox = new QComboBox(this);
    comboBox->addItem("TICHONOV", cv::StereoVar::PENALIZATION_TICHONOV);
    comboBox->setItemData(0, "Linear smoothness.", Qt::ToolTipRole);
    comboBox->addItem("CHARBONNIER", cv::StereoVar::PENALIZATION_CHARBONNIER);
    comboBox->setItemData(1, "Non-linear edge preserving smoothness.", Qt::ToolTipRole);
    comboBox->addItem("PERONA_MALIK", cv::StereoVar::PENALIZATION_PERONA_MALIK);
    comboBox->setItemData(2, "Non-linear edge-enhancing smoothness.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(penalizationChanged(int)));
    layout->addWidget(comboBox, row, 1);
    comboBoxPenalization = comboBox;

    row++;

    // Cycle
    tooltip = "Type of the multigrid cycle (ignored if flag USE_AUTO_PARAMS is set).";
    
    label = new QLabel("Cycle", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    comboBox = new QComboBox(this);
    comboBox->addItem("Null-cycle", cv::StereoVar::CYCLE_O);
    comboBox->setItemData(0, "Null-cycles.", Qt::ToolTipRole);
    comboBox->addItem("V-cycle", cv::StereoVar::CYCLE_V);
    comboBox->setItemData(1, "V-cycles.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(cycleChanged(int)));
    layout->addWidget(comboBox, row, 1);
    comboBoxCycle = comboBox;

    row++;

    // Flags
    QGroupBox *group = new QGroupBox("Flags", this);
    group->setLayout(new QVBoxLayout(group));

    checkBox = new QCheckBox("USE_INITIAL_DISPARITY", this);
    checkBox->setToolTip("Use the input flow as the initial flow approximation.");
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(flagsChanged()));
    group->layout()->addWidget(checkBox);
    checkBoxUseInitialDisparity = checkBox;

    checkBox = new QCheckBox("USE_EQUALIZE_HIST", this);
    checkBox->setToolTip("Use the histogram equalization in the pre-processing phase.");
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(flagsChanged()));
    group->layout()->addWidget(checkBox);
    checkBoxUseEqualizeHist = checkBox;

    checkBox = new QCheckBox("USE_SMART_ID", this);
    checkBox->setToolTip("Use the smart iteration distribution (SID).");
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(flagsChanged()));
    group->layout()->addWidget(checkBox);
    checkBoxUseSmartId = checkBox;

    checkBox = new QCheckBox("USE_AUTO_PARAMS", this);
    checkBox->setToolTip("Allow the method to initialize the main parameters.");
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(flagsChanged()));
    group->layout()->addWidget(checkBox);
    checkBoxUseAutoParams = checkBox;

    checkBox = new QCheckBox("USE_MEDIAN_FILTERING", this);
    checkBox->setToolTip("Use the median filer of the solution in the post-processing phase.");
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(flagsChanged()));
    group->layout()->addWidget(checkBox);
    checkBoxUseMedianFiltering = checkBox;

    layout->addWidget(group, row, 0, 1, 2);

    row++;

    // Spacer for padding
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, 0, 1, 2);


    // Update parameters
    updateParameters();
}

ConfigTabVar::~ConfigTabVar ()
{
}

void ConfigTabVar::presetChanged (int index)
{
    method->usePreset(comboBoxPreset->itemData(index).toInt());
}

void ConfigTabVar::penalizationChanged (int index)
{
    method->setPenalization(comboBoxPenalization->itemData(index).toInt());
}

void ConfigTabVar::cycleChanged (int index)
{
    method->setCycle(comboBoxCycle->itemData(index).toInt());
}

void ConfigTabVar::flagsChanged ()
{
    int flags = 0;
    flags += checkBoxUseInitialDisparity->isChecked() * cv::StereoVar::USE_INITIAL_DISPARITY;
    flags += checkBoxUseEqualizeHist->isChecked() * cv::StereoVar::USE_EQUALIZE_HIST;
    flags += checkBoxUseSmartId->isChecked() * cv::StereoVar::USE_SMART_ID;
    flags += checkBoxUseAutoParams->isChecked() * cv::StereoVar::USE_AUTO_PARAMS;
    flags += checkBoxUseMedianFiltering->isChecked() * cv::StereoVar::USE_MEDIAN_FILTERING;

    method->setFlags(flags);
}

static inline void checkBoxSetCheckedQuiet (QCheckBox *checkBox, bool checked)
{
    bool oldState = checkBox->blockSignals(true);
    checkBox->setChecked(checked);
    checkBox->blockSignals(oldState);
}

void ConfigTabVar::updateParameters ()
{
    spinBoxLevels->setValue(method->getLevels());
    spinBoxPyrScale->setValue(method->getPyrScale());
    spinBoxNumIterations->setValue(method->getNumIterations());

    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxMaxDisparity->setValue(method->getMaxDisparity());

    spinBoxPolyN->setValue(method->getPolyN());
    spinBoxPolySigma->setValue(method->getPolySigma());

    spinBoxFi->setValue(method->getFi());
    spinBoxLambda->setValue(method->getLambda());

    comboBoxPenalization->setCurrentIndex(comboBoxPenalization->findData(method->getPenalization()));
    comboBoxCycle->setCurrentIndex(comboBoxCycle->findData(method->getCycle()));

    int flags = method->getFlags();
    checkBoxSetCheckedQuiet(checkBoxUseInitialDisparity, flags & cv::StereoVar::USE_INITIAL_DISPARITY);
    checkBoxSetCheckedQuiet(checkBoxUseEqualizeHist, flags & cv::StereoVar::USE_EQUALIZE_HIST);
    checkBoxSetCheckedQuiet(checkBoxUseSmartId, flags & cv::StereoVar::USE_SMART_ID);
    checkBoxSetCheckedQuiet(checkBoxUseAutoParams, flags & cv::StereoVar::USE_AUTO_PARAMS);
    checkBoxSetCheckedQuiet(checkBoxUseMedianFiltering, flags & cv::StereoVar::USE_MEDIAN_FILTERING);
}
