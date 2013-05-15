#include "StereoMethodBlockMatching.h"

#include <opencv2/imgproc/imgproc.hpp>


StereoMethodBlockMatching::StereoMethodBlockMatching (QObject *parent)
    : StereoMethod(parent)
{
    shortName = "BM";
    configWidget = new ConfigTabBlockMatching(this);
}

StereoMethodBlockMatching::~StereoMethodBlockMatching ()
{
    //delete configWidget;
}


// *********************************************************************
// *                      Depth image computation                      *
// *********************************************************************
void StereoMethodBlockMatching::computeDepthImage (const cv::Mat &img1, const cv::Mat &img2, cv::Mat &depth)
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

    
    // Compute depth image
    bm(tmpImg1, tmpImg2, tmpDepth);

    // Normalize to output
    tmpDepth.convertTo(depth, CV_8U, 255/(bm.state->numberOfDisparities*16.));
}


// *********************************************************************
// *                         Method parameters                         *
// *********************************************************************
// Pre-filter type
int StereoMethodBlockMatching::getPreFilterType () const
{
    return bm.state->preFilterType;
}

void StereoMethodBlockMatching::setPreFilterType (int newValue)
{
    // Validate
    if (newValue != CV_STEREO_BM_NORMALIZED_RESPONSE && newValue != CV_STEREO_BM_XSOBEL) {
        newValue = CV_STEREO_BM_NORMALIZED_RESPONSE;
    }

    // Set if necessary
    if (bm.state->preFilterType != newValue) {
        bm.state->preFilterType = newValue;
        emit parameterChanged();
    }
}

// Pre-filter size
int StereoMethodBlockMatching::getPreFilterSize () const
{
    return bm.state->preFilterSize;
}

void StereoMethodBlockMatching::setPreFilterSize (int newValue)
{
    // Validate
    newValue += !(newValue % 2); // Must be odd
    newValue = qBound(5, newValue, 255);
    
    // Set if necessary
    if (bm.state->preFilterSize != newValue) {
        bm.state->preFilterSize = newValue;
        emit parameterChanged();
    }
}
    
// Pre-filter clipping
int StereoMethodBlockMatching::getPreFilterCap () const
{
    return bm.state->preFilterCap;
}

void StereoMethodBlockMatching::setPreFilterCap (int newValue)
{
    // Validate
    newValue = qBound(1, newValue, 63);

    // Set if necessary
    if (bm.state->preFilterCap != newValue) {
        bm.state->preFilterCap = newValue;
        emit parameterChanged();
    }
}
        

// Sum-of-absolute difference window size
int StereoMethodBlockMatching::getSADWindowSize () const
{
    return bm.state->SADWindowSize;
}

void StereoMethodBlockMatching::setSADWindowSize (int newValue)
{
    // Validate
    newValue += !(newValue % 2); // Must be odd
    newValue = qBound(5, newValue, 255);

    // Set if necessary
    if (bm.state->SADWindowSize != newValue) {
        bm.state->SADWindowSize = newValue;
        emit parameterChanged();
    }
}
    
// Minimum disparity
int StereoMethodBlockMatching::getMinDisparity () const
{
    return bm.state->minDisparity;
}

void StereoMethodBlockMatching::setMinDisparity (int newValue)
{
    // Set if necessary
    if (bm.state->minDisparity != newValue) {
        bm.state->minDisparity = newValue;
        emit parameterChanged();
    }
}

// Number of disparity levels
int StereoMethodBlockMatching::getNumDisparities () const
{
    return bm.state->numberOfDisparities;
}

void StereoMethodBlockMatching::setNumDisparities (int newValue)
{
    // Validate
    newValue = qRound(newValue / 16.0) * 16; // Must be divisible by 16
    newValue = qMax(16, newValue);
    
    // Set if necessary
    if (bm.state->numberOfDisparities != newValue) {
        bm.state->numberOfDisparities = newValue;
        emit parameterChanged();
    }
}
    
  
// Post-filtering texture threshold
int StereoMethodBlockMatching::getTextureThreshold () const
{
    return bm.state->textureThreshold;
}

void StereoMethodBlockMatching::setTextureThreshold (int newValue)
{
    // Set if necessary
    if (bm.state->textureThreshold != newValue) {
        bm.state->textureThreshold = newValue;
        emit parameterChanged();
    }
}

// Uniqueness ratio; accept disparity d* only if:
//  SAD(d) >= SAD(d*)*(1 + uniquenessRatio/100.)
// for any d!) d* +/- 1 within the search range
int StereoMethodBlockMatching::getUniquenessRatio () const
{
    return bm.state->uniquenessRatio;
}

void StereoMethodBlockMatching::setUniquenessRatio (int newValue)
{
    // Set if necessary
    if (bm.state->uniquenessRatio != newValue) {
        bm.state->uniquenessRatio = newValue;
        emit parameterChanged();
    }
}

// Disparity variantion window
int StereoMethodBlockMatching::getSpeckleWindowSize () const
{
    return bm.state->speckleWindowSize;
}

void StereoMethodBlockMatching::setSpeckleWindowSize (int newValue)
{
    // Set if necessary
    if (bm.state->speckleWindowSize != newValue) {
        bm.state->speckleWindowSize = newValue;
        emit parameterChanged();
    }
}

// Acceptable range of variation in window    
int StereoMethodBlockMatching::getSpeckleRange () const
{
    return bm.state->speckleRange;
}

void StereoMethodBlockMatching::setSpeckleRange (int newValue)
{
    // Set if necessary
    if (bm.state->speckleRange != newValue) {
        bm.state->speckleRange = newValue;
        emit parameterChanged();
    }
}
 
// Whether to try smaller windows or not (more accurate results, but slower)
bool StereoMethodBlockMatching::getTrySmallerWindows () const
{
    return bm.state->trySmallerWindows;
}

void StereoMethodBlockMatching::setTrySmallerWindows (bool newValue)
{
    // Set if necessary
    if (bm.state->trySmallerWindows != newValue) {
        bm.state->trySmallerWindows = newValue;
        emit parameterChanged();
    }
}

// Disp12MaxDiff
int StereoMethodBlockMatching::getDisp12MaxDiff () const
{
    return bm.state->disp12MaxDiff;
}

void StereoMethodBlockMatching::setDisp12MaxDiff (int newValue)
{
    if (bm.state->disp12MaxDiff != newValue) {
        bm.state->disp12MaxDiff = newValue;
        emit parameterChanged();
    }
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabBlockMatching::ConfigTabBlockMatching (StereoMethodBlockMatching *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    QLabel *label;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QFrame *line;
    QCheckBox *checkBox;

    connect(method, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Name
    label = new QLabel("OpenCV block matching", this);
    label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(label, row, 0, 1, 2);

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Pre-filter type
    label = new QLabel("Pre-filter type", this);
    layout->addWidget(label, row, 0);

    comboBox = new QComboBox(this);
    comboBox->addItem("NORMALIZED_RESPONSE", CV_STEREO_BM_NORMALIZED_RESPONSE);
    comboBox->addItem("XSOBEL", CV_STEREO_BM_XSOBEL);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
    layout->addWidget(comboBox, row, 1);
    comboBoxPreFilterType = comboBox;

    row++;

    // Pre-filter size
    label = new QLabel("Pre-filter size", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(5, 255); // 5...255
    spinBox->setSingleStep(2); // Allows only odd values
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPreFilterSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxPreFilterSize = spinBox;

    row++;

    // Pre-filter cap
    label = new QLabel("Pre-filter cap", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 63);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setPreFilterCap(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxPreFilterCap = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // SAD window size
    label = new QLabel("SAD window size", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(5, 255); // 5...255
    spinBox->setSingleStep(2); // Always odd values
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSADWindowSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSADWindowSize = spinBox;

    row++;

    // Min disparity
    label = new QLabel("Min. disparity", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-1000, 1000);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setMinDisparity(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxMinDisparity = spinBox;

    row++;

    // Num disparities
    label = new QLabel("Num. disparities", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(16, 16*1000);
    spinBox->setSingleStep(16); // Must be divisible by 16
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setNumDisparities(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxNumDisparities = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Texture threshold
    label = new QLabel("Texture threshold", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 32000);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setTextureThreshold(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxTextureThreshold = spinBox;

    row++;

    // Uniqueness ratio
    label = new QLabel("Uniqueness ratio", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 255);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setUniquenessRatio(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxUniquenessRatio = spinBox;

    row++;

    // Speckle window size
    label = new QLabel("Speckle window size", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 100);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleWindowSize(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSpeckleWindowSize = spinBox;

    row++;

    // Speckle range
    label = new QLabel("Speckle range", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 100);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setSpeckleRange(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxSpeckleRange = spinBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Try smaller windows
    checkBox = new QCheckBox("Try smaller windows", this);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(trySmallerWindowsChanged(int)));
    layout->addWidget(checkBox, row, 0, 1, 2);
    checkBoxTrySmallerWindow = checkBox;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Disp12MaxDiff
    label = new QLabel("Disp12MaxDiff", this);
    layout->addWidget(label, row, 0);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    connect(spinBox, SIGNAL(valueChanged(int)), method, SLOT(setDisp12MaxDiff(int)));
    layout->addWidget(spinBox, row, 1);
    spinBoxDisp12MaxDiff = spinBox;

    row++;

    // Spring for padding

    // Update parameters
    updateParameters();
}

ConfigTabBlockMatching::~ConfigTabBlockMatching ()
{
}

void ConfigTabBlockMatching::currentIndexChanged (int index)
{
    method->setPreFilterType(comboBoxPreFilterType->itemData(index).toInt());
}

void ConfigTabBlockMatching::trySmallerWindowsChanged (int state)
{
    method->setTrySmallerWindows(state == Qt::Checked);
}


void ConfigTabBlockMatching::updateParameters ()
{
    comboBoxPreFilterType->setCurrentIndex(comboBoxPreFilterType->findData(method->getPreFilterType()));
    
    spinBoxPreFilterSize->setValue(method->getPreFilterSize());
    spinBoxPreFilterCap->setValue(method->getPreFilterCap());
    
    spinBoxSADWindowSize->setValue(method->getSADWindowSize());
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxNumDisparities->setValue(method->getNumDisparities());
    
    spinBoxTextureThreshold->setValue(method->getTextureThreshold());
    spinBoxUniquenessRatio->setValue(method->getUniquenessRatio());
    spinBoxSpeckleWindowSize->setValue(method->getSpeckleWindowSize());
    spinBoxSpeckleRange->setValue(method->getSpeckleRange());
    
    checkBoxTrySmallerWindow->setChecked(method->getTrySmallerWindows());

    spinBoxDisp12MaxDiff->setValue(method->getDisp12MaxDiff());
}
