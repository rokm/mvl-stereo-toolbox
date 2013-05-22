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


StereoMethodConstantSpaceBeliefPropagationGPU::StereoMethodConstantSpaceBeliefPropagationGPU (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "CSBP_GPU";
    configWidget = new ConfigTabConstantSpaceBeliefPropagationGPU(this);

    usePreset(OpenCVInit);
}

StereoMethodConstantSpaceBeliefPropagationGPU::~StereoMethodConstantSpaceBeliefPropagationGPU ()
{
    // Unparent the config widget and destroy it
    configWidget->setParent(0);
    delete configWidget;
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
// *                    Disparity image computation                    *
// *********************************************************************
void StereoMethodConstantSpaceBeliefPropagationGPU::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    cv::gpu::GpuMat gpu_disp;

    if (1) {
        // Make sure that GPU matrices are destroyed as soon as they are
        // not needed anymore via scoping...
        cv::gpu::GpuMat gpu_img1(img1);
        cv::gpu::GpuMat gpu_img2(img2);

        // Compute disparity image
        bp(gpu_img1, gpu_img2, gpu_disp);
    }
    
    // Convert and download
    cv::gpu::GpuMat gpu_disp8u;
    gpu_disp.convertTo(gpu_disp8u, CV_8U);
    gpu_disp8u.download(disparity);

    // Number of disparities
    numDisparities = getNumDisparities();
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
    QFormLayout *layout = new QFormLayout(this);

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

    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Preset
    tooltip = "Presets for quick initialization.";
    
    label = new QLabel("Preset", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("OpenCV - init", StereoMethodConstantSpaceBeliefPropagationGPU::OpenCVInit);
    comboBox->setItemData(0, "Initial OpenCV settings.", Qt::ToolTipRole);
    comboBox->addItem("OpenCV - recommended", StereoMethodConstantSpaceBeliefPropagationGPU::OpenCVRecommended);
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

    // Number of disparity levels on first level
    tooltip = "Number of disparity levels on the first level.";

    label = new QLabel("Num. plane", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNrPlane(int)));
    spinBoxNrPlane = spinBox;

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

    // Minimal disparity threshold
    tooltip = " Minimal disparity threshold.";

    label = new QLabel("Min. disp. thr.", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMinDispThreshold(int)));
    spinBoxMinDispThreshold = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Full DP
    tooltip = "Use local minimum data cost initialization algorithm instead of a global one.";
    
    checkBox = new QCheckBox("Local cost", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(localCostChanged(int)));
    checkBoxUseLocalCost = checkBox;

    layout->addRow(checkBox);

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
