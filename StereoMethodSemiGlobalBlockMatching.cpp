#include "StereoMethodSemiGlobalBlockMatching.h"

#include <opencv2/imgproc/imgproc.hpp>


StereoMethodSemiGlobalBlockMatching::StereoMethodSemiGlobalBlockMatching (QObject *parent)
    : StereoMethod(parent)
{
    // Config interface
    shortName = "SGBM";
    configWidget = new ConfigTabSemiGlobalBlockMatching(this);

    usePreset(OpenCV);
}

StereoMethodSemiGlobalBlockMatching::~StereoMethodSemiGlobalBlockMatching ()
{
    //delete configWidget;
}


void StereoMethodSemiGlobalBlockMatching::usePreset (int type)
{
    switch (type) {
        case OpenCV: {
            // OpenCV
            sgbm = cv::StereoSGBM();
            sgbm.numberOfDisparities = ((imageWidth/8) + 15) & -16;
            break;
        }
        case StereoMatch: {
            // "Stereo match" example
            sgbm = cv::StereoSGBM();

            sgbm.preFilterCap = 63;
            sgbm.SADWindowSize = 3;

            sgbm.P1 = 8*imageChannels*sgbm.SADWindowSize*sgbm.SADWindowSize;
            sgbm.P2 = 32*imageChannels*sgbm.SADWindowSize*sgbm.SADWindowSize;
            sgbm.minDisparity = 0;
            sgbm.numberOfDisparities = ((imageWidth/8) + 15) & -16;
            sgbm.uniquenessRatio = 10;
            sgbm.speckleWindowSize = 100;
            sgbm.speckleRange = 32;
            sgbm.disp12MaxDiff = 1;
            
            break;
        }
    };

    emit parameterChanged();
}


// *********************************************************************
// *                      Depth image computation                      *
// *********************************************************************
void StereoMethodSemiGlobalBlockMatching::computeDepthImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &depth)
{
    sgbm(img1, img2, tmpDepth);

    // Normalize to output
    tmpDepth.convertTo(depth, CV_8U, 255/(sgbm.numberOfDisparities*16.));
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Minimum disparity
int StereoMethodSemiGlobalBlockMatching::getMinDisparity () const
{
    return sgbm.minDisparity;
}

void StereoMethodSemiGlobalBlockMatching::setMinDisparity (int newValue)
{
    setParameter(sgbm.minDisparity, newValue);
}

// Number of diparity levels
int StereoMethodSemiGlobalBlockMatching::getNumDisparities () const
{
    return sgbm.numberOfDisparities;
}

void StereoMethodSemiGlobalBlockMatching::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 16.0) * 16; // Must be divisible by 16
    newValue = qMax(16, newValue);
    
    setParameter(sgbm.numberOfDisparities, newValue);
}

// Sum-of-absolute difference window size
int StereoMethodSemiGlobalBlockMatching::getSADWindowSize () const
{
    return sgbm.SADWindowSize;
}

void StereoMethodSemiGlobalBlockMatching::setSADWindowSize (int newValue)
{
    setParameter(sgbm.SADWindowSize, newValue);
}

// Pre-filter clipping
int StereoMethodSemiGlobalBlockMatching::getPreFilterCap () const
{
    return sgbm.preFilterCap;
}

void StereoMethodSemiGlobalBlockMatching::setPreFilterCap (int newValue)
{
    setParameter(sgbm.preFilterCap, newValue);
}


// Uniqueness ratio; accept disparity d* only if:
//  SAD(d) >= SAD(d*)*(1 + uniquenessRatio/100.)
// for any d!) d* +/- 1 within the search range
int StereoMethodSemiGlobalBlockMatching::getUniquenessRatio () const
{
    return sgbm.uniquenessRatio;
}

void StereoMethodSemiGlobalBlockMatching::setUniquenessRatio (int newValue)
{
    setParameter(sgbm.uniquenessRatio, newValue);
}

// P1
int StereoMethodSemiGlobalBlockMatching::getP1 () const
{
    return sgbm.P1;
}

void StereoMethodSemiGlobalBlockMatching::setP1 (int newValue)
{
    setParameter(sgbm.P1, newValue);
}

// P2    
int StereoMethodSemiGlobalBlockMatching::getP2 () const
{
    return sgbm.P2;
}

void StereoMethodSemiGlobalBlockMatching::setP2 (int newValue)
{
    setParameter(sgbm.P2, newValue);
}

// Disparity variantion window
int StereoMethodSemiGlobalBlockMatching::getSpeckleWindowSize () const
{
    return sgbm.speckleWindowSize;
}

void StereoMethodSemiGlobalBlockMatching::setSpeckleWindowSize (int newValue)
{
    setParameter(sgbm.speckleWindowSize, newValue);
}

// Acceptable range of variation in window    
int StereoMethodSemiGlobalBlockMatching::getSpeckleRange () const
{
    return sgbm.speckleRange;
}

void StereoMethodSemiGlobalBlockMatching::setSpeckleRange (int newValue)
{
    setParameter(sgbm.speckleRange, newValue);
}

// Disp12MaxDiff
int StereoMethodSemiGlobalBlockMatching::getDisp12MaxDiff () const
{
    return sgbm.disp12MaxDiff;
}

void StereoMethodSemiGlobalBlockMatching::setDisp12MaxDiff (int newValue)
{
    setParameter(sgbm.disp12MaxDiff, newValue);
}


// FullDP - double pass? If set to true, method becomes called "HH" in
// stereo match OpenCV example; otherwise, it is SGBM
bool StereoMethodSemiGlobalBlockMatching::getFullDP () const
{
    return sgbm.fullDP;
}

void StereoMethodSemiGlobalBlockMatching::setFullDP (bool newValue)
{
    setParameter(sgbm.fullDP, newValue);
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabSemiGlobalBlockMatching::ConfigTabSemiGlobalBlockMatching (StereoMethodSemiGlobalBlockMatching *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    QLabel *label;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QFrame *line;
    QCheckBox *checkBox;
    QString tooltip;

    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Name
    label = new QLabel("OpenCV semi-global block matching", this);
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
    comboBox->addItem("OpenCV", StereoMethodSemiGlobalBlockMatching::OpenCV);
    comboBox->setItemData(0, "Initial OpenCV settings.", Qt::ToolTipRole);
    comboBox->addItem("StereoMatch", StereoMethodSemiGlobalBlockMatching::StereoMatch);
    comboBox->setItemData(1, "Settings from \"Stereo Match\" example.", Qt::ToolTipRole);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(presetChanged(int)));
    layout->addWidget(comboBox, row, 1);
    comboBoxPreset = comboBox;

    row++;

    // Min disparity
    tooltip = "Minimum possible disparity value. Normally, it is zero but sometimes rectification algorithms \n"
              "can shift images, so this parameter needs to be adjusted accordingly.";
              
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

    // Num disparities
    tooltip = "Maximum disparity minus minimum disparity. The value is always greater than zero. In the current \n"
              "implementation, this parameter must be divisible by 16.";
              
    label = new QLabel("Num. disparities", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(16, 16*1000);
    spinBox->setSingleStep(16); // Must be divisible by 16
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumDisparities(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxNumDisparities = spinBox;

    row++;

    // SAD window size
    tooltip = "Matched block size. It must be an odd number >=1. Normally, it should be somewhere in the 3-11 range.";
    
    label = new QLabel("SAD window size", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 255);
    spinBox->setSingleStep(1); // Always odd values
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSADWindowSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSADWindowSize = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Pre-filter cap
    tooltip = "Truncation value for the prefiltered image pixels. The algorithm first computes x-derivative at each pixel \n"
              "and clips its value by [-preFilterCap, preFilterCap] interval. The result values are passed to the \n"
              "Birchfield-Tomasi pixel cost function.";
              
    label = new QLabel("Pre-filter cap", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 63);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPreFilterCap(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxPreFilterCap = spinBox;

    row++;

    // Uniqueness ratio
    tooltip = "Margin in percentage by which the best (minimum) computed cost function value should \"win\" the second best \n"
              "value to consider the found match correct. Normally, a value within the 5-15 range is good enough.";
    
    label = new QLabel("Uniqueness ratio", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 255);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setUniquenessRatio(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxUniquenessRatio = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // P1
    tooltip = "The first parameter controlling the disparity smoothness. The larger the \n"
              "value is, the smoother the disparity is. P1 is the penalty on the disparity \n"
              "change by plus or minus 1 between neighbor pixels. Algorithm requires P2 > P1. \n"
              "OpenCV stereo example recommends multipliers of numImageChannels*SADWindowSize^2";
              
    label = new QLabel("P1", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setP1(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxP1 = spinBox;

    row++;

    // P2
    tooltip = "The second parameter controlling the disparity smoothness. The larger the \n"
              "value is, the smoother the disparity is. P2 is the penalty on the disparity \n"
              "change by more than 1 between neighbor pixels. The algorithm requires P2 > P1. \n"
              "OpenCV stereo example recommends multipliers of numImageChannels*SADWindowSize^2";
              
    label = new QLabel("P2", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, INT_MAX);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setP2(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxP2 = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Speckle window size
    tooltip = "Maximum size of smooth disparity regions to consider their noise speckles and invalidate. \n"
              "Set it to 0 to disable speckle filtering. Otherwise, set it somewhere in the 50-200 range.";
              
    label = new QLabel("Speckle window size", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 100);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleWindowSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSpeckleWindowSize = spinBox;

    row++;

    // Speckle range
    tooltip = "Maximum disparity variation within each connected component. If you do speckle filtering, set \n"
              "the parameter to a positive value, it will be implicitly multiplied by 16. Normally, 1 or 2 is \n"
              "good enough.";
              
    label = new QLabel("Speckle range", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 200);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleRange(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSpeckleRange = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Disp12MaxDiff
    tooltip = "Maximum allowed difference (in integer pixel units) in the left-right disparity check. \n"
              "Set it to a non-positive value to disable the check.";
    
    label = new QLabel("Disp12MaxDiff", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setDisp12MaxDiff(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxDisp12MaxDiff = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Full DP
    tooltip = "Set it to true to run the full-scale two-pass dynamic programming algorithm. It will consume \n"
              "O(W*H*numDisparities) bytes, which is large for 640x480 stereo and huge for HD-size pictures.";
    
    checkBox = new QCheckBox("Full DP", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(fullDPChanged(int)));
    layout->addWidget(checkBox, row, 0, 1, 2);
    checkBoxFullDP = checkBox;

    row++;

    // Spacer for padding
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, 0, 1, 2);

    // Update parameters
    updateParameters();
}

ConfigTabSemiGlobalBlockMatching::~ConfigTabSemiGlobalBlockMatching ()
{
}

void ConfigTabSemiGlobalBlockMatching::presetChanged (int index)
{
    method->usePreset(comboBoxPreset->itemData(index).toInt());
}

void ConfigTabSemiGlobalBlockMatching::fullDPChanged (int state)
{
    method->setFullDP(state == Qt::Checked);
}


void ConfigTabSemiGlobalBlockMatching::updateParameters ()
{
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxNumDisparities->setValue(method->getNumDisparities());
    spinBoxSADWindowSize->setValue(method->getSADWindowSize());

    spinBoxPreFilterCap->setValue(method->getPreFilterCap());
    
    spinBoxUniquenessRatio->setValue(method->getUniquenessRatio());

    // P1 and P2 require some more work...
    int SADWindowSize = method->getSADWindowSize();
    int numChannels = method->getImageChannels();
    if (SADWindowSize > 0 && numChannels > 0) {
        spinBoxP1->setValue(method->getP1() / (numChannels*SADWindowSize*SADWindowSize));
        spinBoxP2->setValue(method->getP2() / (numChannels*SADWindowSize*SADWindowSize));
    } else {
        spinBoxP1->setValue(method->getP1());
        spinBoxP2->setValue(method->getP2());
    }

    spinBoxSpeckleWindowSize->setValue(method->getSpeckleWindowSize());
    spinBoxSpeckleRange->setValue(method->getSpeckleRange());
    
    spinBoxDisp12MaxDiff->setValue(method->getDisp12MaxDiff());

    checkBoxFullDP->setChecked(method->getFullDP());
}
