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
 
#include "StereoMethodConstantSpaceBeliefPropagationGPU.h"

#include <cfloat>
#include <climits>
#include <opencv2/gpu/gpu.hpp>


StereoMethodConstantSpaceBeliefPropagationGPU::StereoMethodConstantSpaceBeliefPropagationGPU (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "CSBP_GPU";
    configWidget = new ConfigTabConstantSpaceBeliefPropagationGPU(this);

    usePreset(OpenCVInit);
}

StereoMethodConstantSpaceBeliefPropagationGPU::~StereoMethodConstantSpaceBeliefPropagationGPU ()
{
    //delete configWidget;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::usePreset (int type)
{
    switch (type) {
        case OpenCVInit: {
            // OpenCV stock
            bp = cv::gpu::StereoConstantSpaceBP();
            break;
        }
        case OpenCVRecommended: {
            // OpenCV recommended parameters estimation
            bp = cv::gpu::StereoConstantSpaceBP();
            bp.estimateRecommendedParams(imageWidth, imageHeight, bp.ndisp, bp.iters, bp.levels, bp.nr_plane);
            break;
        }
    };

    emit parameterChanged();
}


// *********************************************************************
// *                      Depth image computation                      *
// *********************************************************************
void StereoMethodConstantSpaceBeliefPropagationGPU::computeDepthImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &depth)
{
    // Upload to GPU
    cv::gpu::GpuMat gpu_img1(img1);
    cv::gpu::GpuMat gpu_img2(img2);

    cv::gpu::GpuMat gpu_depth, gpu_depth8u;

    // Compute disparity image
    bp(gpu_img1, gpu_img2, gpu_depth);

    // Convert and download
    gpu_depth.convertTo(gpu_depth8u, CV_8U);
    gpu_depth8u.download(depth);
}


// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethodConstantSpaceBeliefPropagationGPU::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
    // Load parameters
    bp = cv::gpu::StereoConstantSpaceBP();

    storage["NumDisparities"] >> bp.ndisp;
    
    storage["Iterations"] >> bp.iters;
    storage["Levels"] >> bp.levels;
    storage["NrPlane"] >> bp.nr_plane;

    storage["MaxDataTerm"] >> bp.max_data_term;
    storage["DataWeight"] >> bp.data_weight;
    storage["MaxDiscTerm"] >> bp.max_disc_term;
    storage["DiscSingleJump"] >> bp.disc_single_jump;
    storage["MinDispThreshold"] >> bp.min_disp_th;

    storage["UseLocalCost"] >> bp.use_local_init_data_cost;
    
    emit parameterChanged();
}

void StereoMethodConstantSpaceBeliefPropagationGPU::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);

    // Save parameters
    storage << "NumDisparities" << bp.ndisp;
    
    storage << "Iterations" << bp.iters;
    storage << "Levels" << bp.levels;
    storage << "NrPlane" << bp.nr_plane;

    storage << "MaxDataTerm" << bp.max_data_term;
    storage << "DataWeight" << bp.data_weight;
    storage << "MaxDiscTerm" << bp.max_disc_term;
    storage << "DiscSingleJump" << bp.disc_single_jump;
    storage << "MinDispThreshold" << bp.min_disp_th;

    storage << "UseLocalCost" << bp.use_local_init_data_cost;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Number of disparities
int StereoMethodConstantSpaceBeliefPropagationGPU::getNumDisparities () const
{
    return bp.ndisp;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setNumDisparities (int newValue)
{
    setParameter(bp.ndisp, newValue);
}

// Number of iterations
int StereoMethodConstantSpaceBeliefPropagationGPU::getIterations () const
{
    return bp.iters;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setIterations (int newValue)
{
    setParameter(bp.iters, newValue);
}
   
// Levels
int StereoMethodConstantSpaceBeliefPropagationGPU::getLevels () const
{
    return bp.levels;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setLevels (int newValue)
{
    setParameter(bp.levels, newValue);
}

// Number of disparity levels on first level
int StereoMethodConstantSpaceBeliefPropagationGPU::getNrPlane () const
{
    return bp.nr_plane;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setNrPlane (int newValue)
{
    setParameter(bp.nr_plane, newValue);
}

// Max data term
double StereoMethodConstantSpaceBeliefPropagationGPU::getMaxDataTerm () const
{
    return bp.max_data_term;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setMaxDataTerm (double newValue)
{
    setParameter(bp.max_data_term, (float)newValue);
}

// Data weight
double StereoMethodConstantSpaceBeliefPropagationGPU::getDataWeight () const
{
    return bp.data_weight;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setDataWeight (double newValue)
{
    setParameter(bp.data_weight, (float)newValue);
}

// Max discontinuity term
double StereoMethodConstantSpaceBeliefPropagationGPU::getMaxDiscTerm () const
{
    return bp.max_disc_term;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setMaxDiscTerm (double newValue)
{
    setParameter(bp.max_disc_term, (float)newValue);
}

// Single discontinuity jump
double StereoMethodConstantSpaceBeliefPropagationGPU::getDiscSingleJump () const
{
    return bp.disc_single_jump;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setDiscSingleJump (double newValue)
{
    setParameter(bp.disc_single_jump, (float)newValue);
}

// Minimal disparity threshold
int StereoMethodConstantSpaceBeliefPropagationGPU::getMinDispThreshold () const
{
    return bp.min_disp_th;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setMinDispThreshold (int newValue)
{
    setParameter(bp.min_disp_th, newValue);
}

// Use local cost
bool StereoMethodConstantSpaceBeliefPropagationGPU::getUseLocalCost () const
{
    return bp.use_local_init_data_cost;
}

void StereoMethodConstantSpaceBeliefPropagationGPU::setUseLocalCost (bool newValue)
{
    setParameter(bp.use_local_init_data_cost, newValue);
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabConstantSpaceBeliefPropagationGPU::ConfigTabConstantSpaceBeliefPropagationGPU (StereoMethodConstantSpaceBeliefPropagationGPU *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    QLabel *label;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QDoubleSpinBox *spinBoxD;
    QFrame *line;
    QCheckBox *checkBox;

    QString tooltip;

    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Name
    label = new QLabel("<b><u>OpenCV GPU belief propagation</u></b>", this);
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
    comboBox->addItem("OpenCV - init", StereoMethodConstantSpaceBeliefPropagationGPU::OpenCVInit);
    comboBox->setItemData(0, "Initial OpenCV settings.", Qt::ToolTipRole);
    comboBox->addItem("OpenCV - recommended", StereoMethodConstantSpaceBeliefPropagationGPU::OpenCVRecommended);
    comboBox->setItemData(1, "Recommended parameters estimated from image dimensions.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(presetChanged(int)));
    layout->addWidget(comboBox, row, 1);
    comboBoxPreset = comboBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Number of disparities
    tooltip = "Number of disparities.";

    label = new QLabel("Num. disparities", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumDisparities(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxNumDisparities = spinBox;

    row++;

    // Number of iterations
    tooltip = "Number of BP iterations on each level.";

    label = new QLabel("Iterations", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setIterations(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxIterations = spinBox;

    row++;

    // Number of levels
    tooltip = "Number of levels.";

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

    // Number of disparity levels on first level
    tooltip = "Number of disparity levels on the first level.";

    label = new QLabel("Num. plane", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNrPlane(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxNrPlane = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Max data term
    tooltip = "Threshold for data cost truncation.";

    label = new QLabel("Max. data term", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setMaxDataTerm(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxMaxDataTerm = spinBoxD;

    row++;

    // Data weight
    tooltip = "Data weight.";

    label = new QLabel("Data weight", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setDataWeight(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxDataWeight = spinBoxD;

    row++;

    // Max discontinuity term
    tooltip = "Threshold for discontinuity truncation.";

    label = new QLabel("Max. disc. term", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setMaxDiscTerm(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxMaxDiscTerm = spinBoxD;

    row++;

    // Discontinuity single jump
    tooltip = "Discontinuity single jump.";

    label = new QLabel("Disc. single jump", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setDiscSingleJump(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxDiscSingleJump = spinBoxD;

    row++;

    // Minimal disparity threshold
    tooltip = " Minimal disparity threshold.";

    label = new QLabel("Min. disp. thr.", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMinDispThreshold(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxMinDispThreshold = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Full DP
    tooltip = "Use local minimum data cost initialization algorithm instead of a global one.";
    
    checkBox = new QCheckBox("Local cost", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(localCostChanged(int)));
    layout->addWidget(checkBox, row, 0, 1, 2);
    checkBoxUseLocalCost = checkBox;

    row++;

    // Spacer for padding
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, 0, 1, 2);

    // Update parameters
    updateParameters();
}

ConfigTabConstantSpaceBeliefPropagationGPU::~ConfigTabConstantSpaceBeliefPropagationGPU ()
{
}

void ConfigTabConstantSpaceBeliefPropagationGPU::presetChanged (int index)
{
    method->usePreset(comboBoxPreset->itemData(index).toInt());
}

void ConfigTabConstantSpaceBeliefPropagationGPU::localCostChanged (int state)
{
    method->setUseLocalCost(state == Qt::Checked);
}


void ConfigTabConstantSpaceBeliefPropagationGPU::updateParameters ()
{
    spinBoxNumDisparities->setValue(method->getNumDisparities());

    spinBoxIterations->setValue(method->getIterations());
    spinBoxLevels->setValue(method->getLevels());
    spinBoxNrPlane->setValue(method->getNrPlane());

    spinBoxMaxDataTerm->setValue(method->getMaxDataTerm());
    spinBoxDataWeight->setValue(method->getDataWeight());
    spinBoxMaxDiscTerm->setValue(method->getMaxDiscTerm());
    spinBoxDiscSingleJump->setValue(method->getDiscSingleJump());
    spinBoxMinDispThreshold->setValue(method->getMinDispThreshold());

    checkBoxUseLocalCost->setChecked(method->getUseLocalCost());
}
