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
void StereoMethodBlockMatchingGPU::computeDisparityImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &disparity, int &numDisparities)
{
    cv::gpu::GpuMat gpu_img1, gpu_img2, gpu_disp;
    
    // Upload to GPU and convert to grayscale, if needed
    if (img1.channels() == 3) {
        cv::gpu::GpuMat gpu_tmp(img1);
        cv::gpu::cvtColor(gpu_tmp, gpu_img1, CV_RGB2GRAY);
    } else {
        gpu_img1.upload(img1);
    }

    if (img2.channels() == 3) {
        cv::gpu::GpuMat gpu_tmp(img2);
        cv::gpu::cvtColor(gpu_tmp, gpu_img2, CV_RGB2GRAY);
    } else {
        gpu_img2.upload(img2);
    }

    // Compute disparity image
    bm(gpu_img1, gpu_img2, gpu_disp);

    // Download
    gpu_disp.download(disparity);

    // Number of disparities
    numDisparities = getNumDisparities();
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
    QFormLayout *layout = new QFormLayout(this);

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

    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Defaults
    tooltip = "Reset to default OpenCV values.";

    button = new QPushButton("Default values");
    button->setToolTip(tooltip);
    connect(button, SIGNAL(released()), method, SLOT(resetToDefaults()));

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Preset
    tooltip = "Parameter presetting.";
    
    label = new QLabel("Preset", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("BASIC_PRESET", cv::gpu::StereoBM_GPU::BASIC_PRESET);
    comboBox->setItemData(0, "Basic mode without pre-processing.", Qt::ToolTipRole);
    comboBox->addItem("PREFILTER_XSOBEL", cv::gpu::StereoBM_GPU::PREFILTER_XSOBEL);
    comboBox->setItemData(1, "Sobel pre-filtering mode.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(presetChanged(int)));
    comboBoxPreset = comboBox;

    layout->addRow(label, comboBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Num disparities
    tooltip = "Number of disparities. It must be a multiple of 8 and less or equal to 256.";
    
    label = new QLabel("Num. disparities", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(8, 256);
    spinBox->setSingleStep(8); // Must be divisible by 8
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumDisparities(int)));
    spinBoxNumDisparities = spinBox;

    layout->addRow(label, spinBox);

    // Window size
    tooltip = "Block size.";

    label = new QLabel("Window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(2, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setWindowSize(int)));
    spinBoxWindowSize = spinBox;

    layout->addRow(label, spinBox);

    // Average texture threshold
    tooltip = "If avergeTexThreshold != 0 then disparity is set 0 in each point (x,y) where for left image \n"
              "SumOfHorizontalGradiensInWindow(x, y, winSize) < (winSize * winSize) * avergeTexThreshold, \n"
              "i.e. input left image is low textured.";

    label = new QLabel("Avg. texture thr.", this);
    label->setToolTip(tooltip);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setAverageTextureThreshold(double)));
    spinBoxAverageTextureThreshold = spinBoxD;

    layout->addRow(label, spinBoxD);

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
