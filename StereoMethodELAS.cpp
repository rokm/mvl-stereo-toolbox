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

#include "StereoMethodELAS.h"

#include <opencv2/imgproc/imgproc.hpp>


StereoMethodELAS::StereoMethodELAS (QObject *parent)
    : StereoMethod(parent), param(Elas::parameters(Elas::ROBOTICS)), elas(param), returnLeft(true)
{
    shortName = "ELAS";
    configWidget = new ConfigTabELAS(this);

    usePreset(ElasRobotics);
}

StereoMethodELAS::~StereoMethodELAS ()
{
    //delete configWidget;
}


void StereoMethodELAS::usePreset (int type)
{
    switch (type) {
        case ElasRobotics: {
            // Robotics preset
            param = Elas::parameters(Elas::ROBOTICS);
            break;
        }
        case ElasMiddlebury: {
            // Middlebury preset
            param = Elas::parameters(Elas::MIDDLEBURY);
            break;
        }
    };

    elas = Elas(param);
    emit parameterChanged();
}


// *********************************************************************
// *                      Depth image computation                      *
// *********************************************************************
void StereoMethodELAS::computeDepthImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &depth)
{
    // Convert to grayscale
    if (img1.channels() == 3) {
        cv::cvtColor(img1, tmpImg1, CV_RGB2GRAY);
    } else {
        tmpImg1 = img1;
    }

    if (img2.channels() == 3) {
        cv::cvtColor(img2, tmpImg2, CV_RGB2GRAY);
    } else {
        tmpImg2 = img2;
    }


    // Allocate output
    int32_t dims[3] = { tmpImg1.cols, tmpImg2.rows, tmpImg1.step };

    if (param.subsampling) {
        tmpDisp1 = cv::Mat(tmpImg1.rows/2, tmpImg1.cols/2, CV_32FC1);
        tmpDisp2 = cv::Mat(tmpImg2.rows/2, tmpImg2.cols/2, CV_32FC1);
    } else {
        tmpDisp1 = cv::Mat(tmpImg1.rows, tmpImg1.cols, CV_32FC1);
        tmpDisp2 = cv::Mat(tmpImg2.rows, tmpImg2.cols, CV_32FC1);
    }

    // Process
    elas.process(tmpImg1.ptr<uint8_t>(), tmpImg2.ptr<uint8_t>(), tmpDisp1.ptr<float>(), tmpDisp2.ptr<float>(), dims);

    // Convert to output
    if (returnLeft) {
        tmpDisp1.convertTo(depth, CV_8U);
    } else {
        tmpDisp2.convertTo(depth, CV_8U);
    }
}

// *********************************************************************
// *                     Parameter import/export                       *
// *********************************************************************
void StereoMethodELAS::loadParameters (const cv::FileStorage &storage)
{
    // Chain up to parent, which validates the storage
    StereoMethod::loadParameters(storage);
    
    // Load parameters
    param = Elas::parameters(Elas::ROBOTICS);

    storage["MinDisparity"] >> param.disp_min;
    storage["MaxDisparity"] >> param.disp_max;
    
    storage["SupportThreshold"] >> param.support_threshold;
    storage["SupportTexture"] >> param.support_texture;
    storage["CandidateStepSize"] >> param.candidate_stepsize;
    storage["InconsistentWindowSize"] >> param.incon_window_size;
    storage["InconsistentThreshold"] >> param.incon_threshold;
    storage["InconsistentMinSupport"] >> param.incon_min_support;

    storage["AddCorners"] >> param.add_corners;
    storage["GridSize"] >> param.grid_size;
    
    storage["Beta"] >> param.beta;
    storage["Gamma"] >> param.gamma;
    storage["Sigma"] >> param.sigma;
    storage["SigmaRadius"] >> param.sradius;
    
    storage["MatchTexture"] >> param.match_texture;
    storage["LRThreshold"] >> param.lr_threshold;
    
    storage["SpeckleSimThreshold"] >> param.speckle_sim_threshold;
    storage["SpeckleSize"] >> param.speckle_size;
    storage["InterpolationGapWidth"] >> param.ipol_gap_width;

    storage["FilterMedian"] >> param.filter_median;
    storage["FilterAdaptiveMean"] >> param.filter_adaptive_mean;
    storage["PostProcessOnlyLeft"] >> param.postprocess_only_left;
    storage["Subsampling"] >> param.subsampling;
    
    storage["ReturnLeft"] >> returnLeft;
    
    elas = Elas(param);
    emit parameterChanged();
}

void StereoMethodELAS::saveParameters (cv::FileStorage &storage) const
{
    // Chain up to parent, which sets up method name
    StereoMethod::saveParameters(storage);

    // Save parameters
    storage << "MinDisparity" << param.disp_min;
    storage << "MaxDisparity" << param.disp_max;
    
    storage << "SupportThreshold" << param.support_threshold;
    storage << "SupportTexture" << param.support_texture;
    storage << "CandidateStepSize" << param.candidate_stepsize;
    storage << "InconsistentWindowSize" << param.incon_window_size;
    storage << "InconsistentThreshold" << param.incon_threshold;
    storage << "InconsistentMinSupport" << param.incon_min_support;

    storage << "AddCorners" << param.add_corners;
    storage << "GridSize" << param.grid_size;

    storage << "Beta" << param.beta;
    storage << "Gamma" << param.gamma;
    storage << "Sigma" << param.sigma;
    storage << "SigmaRadius" << param.sradius;
    
    storage << "MatchTexture" << param.match_texture;
    storage << "LRThreshold" << param.lr_threshold;
    
    storage << "SpeckleSimThreshold" << param.speckle_sim_threshold;
    storage << "SpeckleSize" << param.speckle_size;
    storage << "InterpolationGapWidth" << param.ipol_gap_width;
    
    storage << "FilterMedian" << param.filter_median;
    storage << "FilterAdaptiveMean" << param.filter_adaptive_mean;
    storage << "PostProcessOnlyLeft" << param.postprocess_only_left;
    storage << "Subsampling" << param.subsampling;

    storage << "ReturnLeft" << returnLeft;
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Minimum disparity
int StereoMethodELAS::getMinDisparity () const
{
    return param.disp_min;
}

void StereoMethodELAS::setMinDisparity (int newValue)
{
    setParameter(param.disp_min, newValue);
}

// Maximum disparity
int StereoMethodELAS::getMaxDisparity () const
{
    return param.disp_max;
}

void StereoMethodELAS::setMaxDisparity (int newValue)
{
    setParameter(param.disp_max, newValue);
}


// Support threshold
double StereoMethodELAS::getSupportThreshold () const
{
    return param.support_threshold;
}

void StereoMethodELAS::setSupportThreshold (double newValue)
{
    setParameter(param.support_threshold, (float)newValue);
}

// Support texture
int StereoMethodELAS::getSupportTexture () const
{
    return param.support_texture;
}

void StereoMethodELAS::setSupportTexture (int newValue)
{
    setParameter(param.support_texture, newValue);
}

// Candidate step size
int StereoMethodELAS::getCandidateStepSize () const
{
    return param.candidate_stepsize;
}

void StereoMethodELAS::setCandidateStepSize (int newValue)
{
    setParameter(param.candidate_stepsize, newValue);
}

// Inconsistent window size
int StereoMethodELAS::getInconsistentWindowSize () const
{
    return param.incon_window_size;
}

void StereoMethodELAS::setInconsistentWindowSize (int newValue)
{
    setParameter(param.incon_window_size, newValue);
}

// Inconsistent threshold
int StereoMethodELAS::getInconsistentThreshold () const
{
    return param.incon_threshold;
}

void StereoMethodELAS::setInconsistentThreshold (int newValue)
{
    setParameter(param.incon_threshold, newValue);
}

// Inconsistent min support
int StereoMethodELAS::getInconsistentMinSupport () const
{
    return param.incon_min_support;
}

void StereoMethodELAS::setInconsistentMinSupport (int newValue)
{
    setParameter(param.incon_min_support, newValue);
}


// Add corners
bool StereoMethodELAS::getAddCorners () const
{
    return param.add_corners;
}

void StereoMethodELAS::setAddCorners (bool newValue)
{
    setParameter(param.add_corners, newValue);
}

// Grid size
int StereoMethodELAS::getGridSize () const
{
    return param.grid_size;
}

void StereoMethodELAS::setGridSize (int newValue)
{
    setParameter(param.grid_size, newValue);
}


// Beta
double StereoMethodELAS::getBeta () const
{
    return param.beta;
}

void StereoMethodELAS::setBeta (double newValue)
{
    setParameter(param.beta, (float)newValue);
}

// Gamma
double StereoMethodELAS::getGamma () const
{
    return param.gamma;
}

void StereoMethodELAS::setGamma (double newValue)
{
    setParameter(param.gamma, (float)newValue);
}

// Sigma
double StereoMethodELAS::getSigma () const
{
    return param.sigma;
}

void StereoMethodELAS::setSigma (double newValue)
{
    setParameter(param.sigma, (float)newValue);
}

// Sigma radius
double StereoMethodELAS::getSigmaRadius () const
{
    return param.sradius;
}

void StereoMethodELAS::setSigmaRadius (double newValue)
{
    setParameter(param.sradius, (float)newValue);
}


// Match texture
int StereoMethodELAS::getMatchTexture () const
{
    return param.match_texture;
}

void StereoMethodELAS::setMatchTexture (int newValue)
{
    setParameter(param.match_texture, newValue);
}

// Left/right threshold
int StereoMethodELAS::getLRThreshold () const
{
    return param.lr_threshold;
}

void StereoMethodELAS::setLRThreshold (int newValue)
{
    setParameter(param.lr_threshold, newValue);
}


// Speckle similarity threshold
double StereoMethodELAS::getSpeckleSimThreshold () const
{
    return param.speckle_sim_threshold;
}

void StereoMethodELAS::setSpeckleSimThreshold (double newValue)
{
    setParameter(param.speckle_sim_threshold, (float)newValue);
}

// Speckle size
int StereoMethodELAS::getSpeckleSize () const
{
    return param.speckle_size;
}

void StereoMethodELAS::setSpeckleSize (int newValue)
{
    setParameter(param.speckle_size, newValue);
}

// Interpolation gap width
int StereoMethodELAS::getInterpolationGapWidth () const
{
    return param.ipol_gap_width;
}

void StereoMethodELAS::setInterpolationGapWidth (int newValue)
{
    setParameter(param.ipol_gap_width, newValue);
}


// Median filter
bool StereoMethodELAS::getFilterMedian () const
{
    return param.filter_median;
}

void StereoMethodELAS::setFilterMedian (bool newValue)
{
    setParameter(param.filter_median, newValue);
}

// Adaptive mean filter
bool StereoMethodELAS::getFilterAdaptiveMean () const
{
    return param.filter_adaptive_mean;
}

void StereoMethodELAS::setFilterAdaptiveMean (bool newValue)
{
    setParameter(param.filter_adaptive_mean, newValue);
}

// Post-process only left
bool StereoMethodELAS::getPostProcessOnlyLeft () const
{
    return param.postprocess_only_left;
}

void StereoMethodELAS::setPostProcessOnlyLeft (bool newValue)
{
    setParameter(param.postprocess_only_left, newValue);
}

// Subsampling
bool StereoMethodELAS::getSubsampling () const
{
    return param.subsampling;
}

void StereoMethodELAS::setSubsampling (bool newValue)
{
    setParameter(param.subsampling, newValue);
}


// Return left/right image
bool StereoMethodELAS::getReturnLeft () const
{
    return returnLeft;
}

void StereoMethodELAS::setReturnLeft (bool newValue)
{
    setParameter(returnLeft, newValue);
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabELAS::ConfigTabELAS (StereoMethodELAS *m, QWidget *parent)
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
    label = new QLabel("<b><u>Efficient LArge-scale Stereo</u></b>", this);
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
    comboBox->addItem("Robotics", StereoMethodELAS::ElasRobotics);
    comboBox->setItemData(0, "Default settings in robotics environment. Does not produce results in \nhalf-occluded areas and is a bit more robust to lighting, etc.", Qt::ToolTipRole);
    comboBox->addItem("Middlebury", StereoMethodELAS::ElasMiddlebury);
    comboBox->setItemData(1, "Default settings for Middlebury benchmark. Interpolates all missing disparities.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(presetChanged(int)));
    layout->addWidget(comboBox, row, 1);
    comboBoxPreset = comboBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Min disparity
    tooltip = "Minimum possible disparity value.";

    label = new QLabel("Min. disparity", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMinDisparity(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxMinDisparity = spinBox;

    row++;

    // Max disparity
    tooltip = "Maximum possible disparity value.";

    label = new QLabel("Min. disparity", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMaxDisparity(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxMaxDisparity = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Support threshold
    tooltip = "Max. uniqueness ratio (best vs. second best support match).";

    label = new QLabel("Support threshold", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setSupportThreshold(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxSupportThreshold = spinBoxD;

    row++;

    // Support texture
    tooltip = "Min. texture for support points.";

    label = new QLabel("Support texture", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSupportTexture(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSupportTexture = spinBox;

    row++;

    // Candidate step size
    tooltip = "Step size of regular grid on which support points are matched.";

    label = new QLabel("Candidate step size", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setCandidateStepSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxCandidateStepSize = spinBox;

    row++;

    // Inconsistent window size
    tooltip = "Window size of inconsistent support point check";

    label = new QLabel("Incon. window size", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setInconsistentWindowSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxInconsistentWindowSize = spinBox;

    row++;

    // Inconsistent threshold
    tooltip = "Disparity similarity threshold for support point to be considered consistent.";

    label = new QLabel("Incon. threshold", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setInconsistentThreshold(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxInconsistentThreshold = spinBox;

    row++;

    // Inconsistent min support
    tooltip = "Minimum number of consistent support points.";

    label = new QLabel("Incon. min. support", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-INT_MAX, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setInconsistentMinSupport(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxInconsistentMinSupport = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Add corners
    tooltip = "Add support points at image corners with nearest neighbor disparities.";
    
    checkBox = new QCheckBox("Add corners", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(addCornersChanged(int)));
    layout->addWidget(checkBox, row, 0, 1, 2);
    checkBoxAddCorners = checkBox;

    row++;

    // Grid size
    tooltip = "Size of neighborhood for additional support point extrapolation.";

    label = new QLabel("Grid size", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setGridSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxGridSize = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;
    
    // Beta
    tooltip = "Image likelihood parameter.";

    label = new QLabel("Beta", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setBeta(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxBeta = spinBoxD;

    row++;
    
    // Gamma
    tooltip = "Prior constant.";

    label = new QLabel("Gamma", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setGamma(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxGamma = spinBoxD;

    row++;

    // Sigma
    tooltip = "Prior sigma.";

    label = new QLabel("Sigma", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setSigma(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxSigma = spinBoxD;

    row++;

    // Sigma radius
    tooltip = "Prior sigma radius.";

    label = new QLabel("Sigma radius", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setSigmaRadius(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxSigmaRadius = spinBoxD;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Match texture
    tooltip = "Min texture for dense matching.";

    label = new QLabel("Match texture", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMatchTexture(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxMatchTexture = spinBox;

    row++;

    // Left/right threshold
    tooltip = "Disparity threshold for left/right consistency check.";

    label = new QLabel("Left/right thr.", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setLRThreshold(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxLRThreshold = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Speckle simimarity threshold
    tooltip = "Similarity threshold for speckle segmentation.";

    label = new QLabel("Speckle sim. thr.", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBoxD = new QDoubleSpinBox(this);
    spinBoxD->setKeyboardTracking(false);
    spinBoxD->setRange(0.0, DBL_MAX);
    connect(spinBoxD, SIGNAL(valueChanged(double)), method, SLOT(setSpeckleSimThreshold(double)));
    layout->addWidget(spinBoxD, row, 1);
    spinBoxSpeckleSimThreshold = spinBoxD;

    row++;

    // Speckle size
    tooltip = "Maximal size of a speckle (small speckles are removed).";

    label = new QLabel("Speckle size", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSpeckleSize = spinBox;

    row++;

    // Interpolation gap width
    tooltip = "Interpolate small gaps (left<->right, top<->bottom).";

    label = new QLabel("Interp. gap width", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setInterpolationGapWidth(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxInterpolationGapWidth = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Median filter
    tooltip = "Optional median filter (approximated).";
    
    checkBox = new QCheckBox("Median filter", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(medianFilterChanged(int)));
    layout->addWidget(checkBox, row, 0, 1, 2);
    checkBoxFilterMedian = checkBox;

    row++;

    // Adaptive mean filter
    tooltip = "Optional adaptive mean filter (approximated).";
    
    checkBox = new QCheckBox("Adaptive mean filter", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(adaptiveMeanFilterChanged(int)));
    layout->addWidget(checkBox, row, 0, 1, 2);
    checkBoxFilterAdaptiveMean = checkBox;

    row++;

    // Postprocess only left
    tooltip = "Save time by not post-processing the right disparity image.";
    
    checkBox = new QCheckBox("Post-process only left", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(postProcessOnlyLeftChanged(int)));
    layout->addWidget(checkBox, row, 0, 1, 2);
    checkBoxPostProcessOnlyLeft = checkBox;

    row++;

    // Subsampling
    tooltip = "Save time by only computing disparities for each 2nd pixel.";
    
    checkBox = new QCheckBox("Subsampling", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(subsamplingChanged(int)));
    layout->addWidget(checkBox, row, 0, 1, 2);
    checkBoxSubsampling = checkBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Return left
    tooltip = "If checked, method returns left disparity image. If unchecked, right disparity image is returned.";
    
    checkBox = new QCheckBox("Return left disp. image", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(returnLeftChanged(int)));
    layout->addWidget(checkBox, row, 0, 1, 2);
    checkBoxReturnLeft = checkBox;

    row++;
    
    // Spacer for padding
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, 0, 1, 2);

    // Update parameters
    updateParameters();
}

ConfigTabELAS::~ConfigTabELAS ()
{
}

void ConfigTabELAS::presetChanged (int index)
{
    method->usePreset(comboBoxPreset->itemData(index).toInt());
}

void ConfigTabELAS::addCornersChanged (int state)
{
    method->setAddCorners(state == Qt::Checked);
}

void ConfigTabELAS::medianFilterChanged (int state)
{
    method->setFilterMedian(state == Qt::Checked);
}

void ConfigTabELAS::adaptiveMeanFilterChanged (int state)
{
    method->setFilterAdaptiveMean(state == Qt::Checked);
}

void ConfigTabELAS::postProcessOnlyLeftChanged (int state)
{
    method->setPostProcessOnlyLeft(state == Qt::Checked);
}

void ConfigTabELAS::subsamplingChanged (int state)
{
    method->setSubsampling(state == Qt::Checked);
}

void ConfigTabELAS::returnLeftChanged (int state)
{
    method->setReturnLeft(state == Qt::Checked);
}

void ConfigTabELAS::updateParameters ()
{
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxMaxDisparity->setValue(method->getMaxDisparity());

    spinBoxSupportThreshold->setValue(method->getSupportThreshold());
    spinBoxSupportTexture->setValue(method->getSupportTexture());
    spinBoxCandidateStepSize->setValue(method->getCandidateStepSize());
    spinBoxInconsistentWindowSize->setValue(method->getInconsistentWindowSize());
    spinBoxInconsistentThreshold->setValue(method->getInconsistentThreshold());
    spinBoxInconsistentMinSupport->setValue(method->getInconsistentMinSupport());

    checkBoxAddCorners->setChecked(method->getAddCorners());
    spinBoxGridSize->setValue(method->getGridSize());
    
    spinBoxBeta->setValue(method->getBeta());
    spinBoxGamma->setValue(method->getGamma());
    spinBoxSigma->setValue(method->getSigma());
    spinBoxSigmaRadius->setValue(method->getSigmaRadius());

    spinBoxMatchTexture->setValue(method->getMatchTexture());
    spinBoxLRThreshold->setValue(method->getLRThreshold());

    spinBoxSpeckleSimThreshold->setValue(method->getSpeckleSimThreshold());
    spinBoxSpeckleSize->setValue(method->getSpeckleSize());
    spinBoxInterpolationGapWidth->setValue(method->getInterpolationGapWidth());

    checkBoxFilterMedian->setChecked(method->getFilterMedian());
    checkBoxFilterAdaptiveMean->setChecked(method->getFilterAdaptiveMean());
    checkBoxPostProcessOnlyLeft->setChecked(method->getPostProcessOnlyLeft());
    checkBoxSubsampling->setChecked(method->getSubsampling());

    checkBoxReturnLeft->setChecked(method->getReturnLeft());
}
