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
 
#include "StereoMethodBlockMatchingGPU.h"

#include <cfloat>
#include <climits>
#include <opencv2/gpu/gpu.hpp>


StereoMethodBlockMatchingGPU::StereoMethodBlockMatchingGPU (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "BM_GPU";
    configWidget = new ConfigTabBlockMatchingGPU(this);
}

StereoMethodBlockMatchingGPU::~StereoMethodBlockMatchingGPU ()
{
    //delete configWidget;
}

void StereoMethodBlockMatchingGPU::resetToDefaults ()
{
    bm = cv::gpu::StereoBM_GPU();
    emit parameterChanged();
}


// *********************************************************************
// *                    Disparity image computation                    *
// *********************************************************************
void StereoMethodBlockMatchingGPU::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &depth)
{
    // Upload to GPU
    cv::gpu::GpuMat gpu_img1(img1);
    cv::gpu::GpuMat gpu_img2(img2);

    cv::gpu::GpuMat gpu_tmp1;
    cv::gpu::GpuMat gpu_tmp2;
    cv::gpu::GpuMat gpu_depth;

    // Convert to grayscale
    if (gpu_img1.channels() == 3) {
        cv::gpu::cvtColor(gpu_img1, gpu_tmp1, CV_RGB2GRAY);
    } else {
        gpu_tmp1 = gpu_img1;
    }

    if (gpu_img2.channels() == 3) {
        cv::gpu::cvtColor(gpu_img2, gpu_tmp2, CV_RGB2GRAY);
    } else {
        gpu_tmp2 = gpu_img2;
    }

    // Compute disparity image
    bm(gpu_tmp1, gpu_tmp2, gpu_depth);

    // Download
    gpu_depth.download(depth);
}

// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethodBlockMatchingGPU::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
    // Load parameters
    bm = cv::gpu::StereoBM_GPU();
    
    storage["Preset"] >> bm.preset;
    storage["NumDisparities"] >> bm.ndisp;
    storage["WindowSize"] >> bm.winSize;
    storage["AverageTextureThreshold"] >> bm.avergeTexThreshold;
    
    emit parameterChanged();
}

void StereoMethodBlockMatchingGPU::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);

    // Save parameters
    storage << "Preset" << bm.preset;
    storage << "NumDisparities" << bm.ndisp;
    storage << "WindowSize" << bm.winSize;
    storage << "AverageTextureThreshold" << bm.avergeTexThreshold;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Preset
int StereoMethodBlockMatchingGPU::getPreset () const
{
    return bm.preset;
}

void StereoMethodBlockMatchingGPU::setPreset (int newValue)
{
    // Validate
    if (newValue != cv::gpu::StereoBM_GPU::BASIC_PRESET && newValue != cv::gpu::StereoBM_GPU::PREFILTER_XSOBEL) {
        newValue = cv::gpu::StereoBM_GPU::BASIC_PRESET;
    }

    setParameter(bm.preset, newValue);
}

// Number of disparities
int StereoMethodBlockMatchingGPU::getNumDisparities () const
{
    return bm.ndisp;
}

void StereoMethodBlockMatchingGPU::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 8.0) * 8; // Must be divisible by 8
    newValue = qMax(8, newValue);

    setParameter(bm.ndisp, newValue);
}
    
// Window size
int StereoMethodBlockMatchingGPU::getWindowSize () const
{
    return bm.winSize;
}

void StereoMethodBlockMatchingGPU::setWindowSize (int newValue)
{
    setParameter(bm.winSize, newValue);
}
        

// Average texture threshold
double StereoMethodBlockMatchingGPU::getAverageTextureThreshold () const
{
    return bm.avergeTexThreshold;
}

void StereoMethodBlockMatchingGPU::setAverageTextureThreshold (double newValue)
{
    setParameter(bm.avergeTexThreshold, (float)newValue);
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabBlockMatchingGPU::ConfigTabBlockMatchingGPU (StereoMethodBlockMatchingGPU *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    QLabel *label;
    QPushButton *button;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QDoubleSpinBox *spinBoxD;
    QFrame *line;

    QString tooltip;

    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Name
    label = new QLabel("<b><u>OpenCV GPU block matching</u></b>", this);
    label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(label, row, 0, 1, 2);

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Defaults
    tooltip = "Reset to default OpenCV values.";

    button = new QPushButton("Default values");
    button->setToolTip(tooltip);
    connect(button, SIGNAL(released()), method, SLOT(resetToDefaults()));
    layout->addWidget(button, row, 0, 1, 2);

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Preset
    tooltip = "Parameter presetting.";
    
    label = new QLabel("Preset", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    comboBox = new QComboBox(this);
    comboBox->addItem("BASIC_PRESET", cv::gpu::StereoBM_GPU::BASIC_PRESET);
    comboBox->setItemData(0, "Basic mode without pre-processing.", Qt::ToolTipRole);
    comboBox->addItem("PREFILTER_XSOBEL", cv::gpu::StereoBM_GPU::PREFILTER_XSOBEL);
    comboBox->setItemData(1, "Sobel pre-filtering mode.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(presetChanged(int)));
    layout->addWidget(comboBox, row, 1);
    comboBoxPreset = comboBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Num disparities
    tooltip = "Number of disparities. It must be a multiple of 8 and less or equal to 256.";
    
    label = new QLabel("Num. disparities", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(8, 256);
    spinBox->setSingleStep(8); // Must be divisible by 8
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumDisparities(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxNumDisparities = spinBox;

    row++;

    // Window size
    tooltip = "Block size.";

    label = new QLabel("Window size", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(2, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setWindowSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxWindowSize = spinBox;

    row++;

    // Average texture threshold
    tooltip = "If avergeTexThreshold != 0 then disparity is set 0 in each point (x,y) where for left image \n"
              "SumOfHorizontalGradiensInWindow(x, y, winSize) < (winSize * winSize) * avergeTexThreshold, \n"
              "i.e. input left image is low textured.";

    label = new QLabel("Avg. texture thr.", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setAverageTextureThreshold(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxAverageTextureThreshold = spinBoxD;

    row++;

    // Spacer for padding
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, 0, 1, 2);

    // Update parameters
    updateParameters();
}

ConfigTabBlockMatchingGPU::~ConfigTabBlockMatchingGPU ()
{
}

void ConfigTabBlockMatchingGPU::presetChanged (int index)
{
    method->setPreset(comboBoxPreset->itemData(index).toInt());
}


void ConfigTabBlockMatchingGPU::updateParameters ()
{
    comboBoxPreset->setCurrentIndex(comboBoxPreset->findData(method->getPreset()));
    
    spinBoxNumDisparities->setValue(method->getNumDisparities());
    spinBoxWindowSize->setValue(method->getWindowSize());
    spinBoxAverageTextureThreshold->setValue(method->getAverageTextureThreshold());
}
