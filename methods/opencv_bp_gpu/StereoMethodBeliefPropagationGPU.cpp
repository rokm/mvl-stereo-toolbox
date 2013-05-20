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
 
#include "StereoMethodBeliefPropagationGPU.h"

#include <cfloat>
#include <climits>
#include <opencv2/gpu/gpu.hpp>


StereoMethodBeliefPropagationGPU::StereoMethodBeliefPropagationGPU (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "BP_GPU";
    configWidget = new ConfigTabBeliefPropagationGPU(this);
}

StereoMethodBeliefPropagationGPU::~StereoMethodBeliefPropagationGPU ()
{
    //delete configWidget;
}

void StereoMethodBeliefPropagationGPU::usePreset (int type)
{
    switch (type) {
        case OpenCVInit: {
            // OpenCV stock
            bp = cv::gpu::StereoBeliefPropagation();
            break;
        }
        case OpenCVRecommended: {
            // OpenCV recommended parameters estimation
            bp = cv::gpu::StereoBeliefPropagation();
            bp.estimateRecommendedParams(imageWidth, imageHeight, bp.ndisp, bp.iters, bp.levels);
            break;
        }
    };

    emit parameterChanged();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void StereoMethodBeliefPropagationGPU::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
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
void StereoMethodBeliefPropagationGPU::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
    // Load parameters
    bp = cv::gpu::StereoBeliefPropagation();
    
    storage["NumDisparities"] >> bp.ndisp;
    
    storage["Iterations"] >> bp.iters;
    storage["Levels"] >> bp.levels;
    
    storage["MaxDataTerm"] >> bp.max_data_term;
    storage["DataWeight"] >> bp.data_weight;
    storage["MaxDiscTerm"] >> bp.max_disc_term;
    storage["DiscSingleJump"] >> bp.disc_single_jump;

    emit parameterChanged();
}

void StereoMethodBeliefPropagationGPU::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);
    
    storage << "NumDisparities" << bp.ndisp;

    storage << "Iterations" << bp.iters;
    storage << "Levels" << bp.levels;
    
    storage << "MaxDataTerm" << bp.max_data_term;
    storage << "DataWeight" << bp.data_weight;
    storage << "MaxDiscTerm" << bp.max_disc_term;
    storage << "DiscSingleJump" << bp.disc_single_jump;  
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Number of disparities
int StereoMethodBeliefPropagationGPU::getNumDisparities () const
{
    return bp.ndisp;
}

void StereoMethodBeliefPropagationGPU::setNumDisparities (int newValue)
{
    setParameter(bp.ndisp, newValue);
}

// Number of iterations
int StereoMethodBeliefPropagationGPU::getIterations () const
{
    return bp.iters;
}

void StereoMethodBeliefPropagationGPU::setIterations (int newValue)
{
    setParameter(bp.iters, newValue);
}
    
// Levels
int StereoMethodBeliefPropagationGPU::getLevels () const
{
    return bp.levels;
}

void StereoMethodBeliefPropagationGPU::setLevels (int newValue)
{
    setParameter(bp.levels, newValue);
}
        
// Max data term
double StereoMethodBeliefPropagationGPU::getMaxDataTerm () const
{
    return bp.max_data_term;
}

void StereoMethodBeliefPropagationGPU::setMaxDataTerm (double newValue)
{
    setParameter(bp.max_data_term, (float)newValue);
}

// Data weight
double StereoMethodBeliefPropagationGPU::getDataWeight () const
{
    return bp.data_weight;
}

void StereoMethodBeliefPropagationGPU::setDataWeight (double newValue)
{
    setParameter(bp.data_weight, (float)newValue);
}

// Max discontinuity term
double StereoMethodBeliefPropagationGPU::getMaxDiscTerm () const
{
    return bp.max_disc_term;
}

void StereoMethodBeliefPropagationGPU::setMaxDiscTerm (double newValue)
{
    setParameter(bp.max_disc_term, (float)newValue);
}

// Single discontinuity jump
double StereoMethodBeliefPropagationGPU::getDiscSingleJump () const
{
    return bp.disc_single_jump;
}

void StereoMethodBeliefPropagationGPU::setDiscSingleJump (double newValue)
{
    setParameter(bp.disc_single_jump, (float)newValue);
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabBeliefPropagationGPU::ConfigTabBeliefPropagationGPU (StereoMethodBeliefPropagationGPU *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    QLabel *label;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QDoubleSpinBox *spinBoxD;
    QFrame *line;

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
    comboBox->addItem("OpenCV - init", StereoMethodBeliefPropagationGPU::OpenCVInit);
    comboBox->setItemData(0, "Initial OpenCV settings.", Qt::ToolTipRole);
    comboBox->addItem("OpenCV - recommended", StereoMethodBeliefPropagationGPU::OpenCVRecommended);
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

    // Spacer for padding
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, 0, 1, 2);

    // Update parameters
    updateParameters();
}

ConfigTabBeliefPropagationGPU::~ConfigTabBeliefPropagationGPU ()
{
}

void ConfigTabBeliefPropagationGPU::presetChanged (int index)
{
    method->usePreset(comboBoxPreset->itemData(index).toInt());
}


void ConfigTabBeliefPropagationGPU::updateParameters ()
{
    spinBoxNumDisparities->setValue(method->getNumDisparities());

    spinBoxIterations->setValue(method->getIterations());
    spinBoxLevels->setValue(method->getLevels());

    spinBoxMaxDataTerm->setValue(method->getMaxDataTerm());
    spinBoxDataWeight->setValue(method->getDataWeight());
    spinBoxMaxDiscTerm->setValue(method->getMaxDiscTerm());
    spinBoxDiscSingleJump->setValue(method->getDiscSingleJump());
}
