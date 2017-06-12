/*
 * OpenCV Binary-descriptor-based Block Matching: method widget
 * Copyright (C) 2017 Rok Mandeljc
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


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace StereoMethodOpenCvBinaryBm {


MethodWidget::MethodWidget (Method *m, QWidget *parent)
    : QWidget(parent), method(m)
{
    connect(method, &Method::parameterChanged, this, &MethodWidget::updateParameters);

    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QPushButton *button;
    QComboBox *comboBox;
    QSpinBox *spinBox;
    QCheckBox *checkBox;
    QFrame *line;

    QString tooltip;

    // Name
    label = new QLabel("<b><u>OpenCV binary block matching</u></b>", this);
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


    // Defaults
    tooltip = "Reset to default OpenCV values.";

    button = new QPushButton("Reset");
    button->setToolTip(tooltip);
    connect(button, &QPushButton::clicked, method, &Method::resetParameters);

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Minimum disparity
    tooltip = "Minimum possible disparity value. Normally, it is zero but sometimes rectification \n"
              "algorithms can shift images, so this parameter needs to be adjusted accordingly.";

    label = new QLabel("Min. disparity", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-9999, 9999);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setMinDisparity);
    spinBoxMinDisparity = spinBox;

    layout->addRow(label, spinBox);

    // Number of disparities
    tooltip = "Maximum disparity minus minimum disparity. The value is always greater than zero. In \n"
              "the current implementation, this parameter must be divisible by 16.";

    label = new QLabel("Num. disparities", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(16, 16*1000);
    spinBox->setSingleStep(16); // Must be divisible by 16
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setNumDisparities);
    spinBoxNumDisparities = spinBox;

    layout->addRow(label, spinBox);

    // Block size
    tooltip = "The size of blocks used in matching. Must be an odd number.";

    label = new QLabel("Block size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(5, 255); // 5...255
    spinBox->setSingleStep(2); // Always odd values
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setBlockSize);
    spinBoxBlockSize = spinBox;

    layout->addRow(label, spinBox);

    // Speckle window size
    tooltip = "The maximum area of speckles to remove (set to 0 to disable speckle filtering).";

    label = new QLabel("Speckle window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 200);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setSpeckleWindowSize);
    spinBoxSpeckleWindowSize = spinBox;

    layout->addRow(label, spinBox);

    // Speckle range
    tooltip = "Acceptable range of disparity variation in each connected component.";

    label = new QLabel("Speckle range", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 100);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setSpeckleRange);
    spinBoxSpeckleRange = spinBox;

    layout->addRow(label, spinBox);

    // Disp12MaxDiff
    tooltip = "Maximum allowed difference (in integer pixel units) in the left-right disparity check. \n"
              "Set it to a non-positive value to disable the check.";

    label = new QLabel("Disp12MaxDiff", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(-1, 255);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setDisp12MaxDiff);
    spinBoxDisp12MaxDiff = spinBox;

    layout->addRow(label, spinBox);


    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Use pre-filter
    tooltip = "";

    checkBox = new QCheckBox("Use pre-filter", this);
    checkBox->setToolTip(tooltip);
    connect(checkBox, &QCheckBox::toggled, method, &Method::setUsePreFilter);
    checkBoxUsePreFilter = checkBox;

    layout->addRow(checkBox);

    // Pre-filter type
    tooltip = "Type of pre-filter used for normalizing input images.";

    label = new QLabel("Pre-filter type", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("NORMALIZED_RESPONSE", cv::stereo::StereoBinaryBM::PREFILTER_NORMALIZED_RESPONSE);
    comboBox->setItemData(0, "Normalized response filter.", Qt::ToolTipRole);
    comboBox->addItem("XSOBEL", cv::stereo::StereoBinaryBM::PREFILTER_XSOBEL);
    comboBox->setItemData(0, "Sobel filter.", Qt::ToolTipRole);

    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this, comboBox] (int index) {
        method->setPreFilterType(comboBox->itemData(index).toInt());
    });

    comboBoxPreFilterType = comboBox;

    layout->addRow(label, comboBox);

    // Pre-filter size
    tooltip = "Pre-filter size; ~5x5 ... 21x21";

    label = new QLabel("Pre-filter size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(5, 255); // 5...255
    spinBox->setSingleStep(2); // Allows only odd values
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setPreFilterSize);
    spinBoxPreFilterSize = spinBox;

    layout->addRow(label, spinBox);

    // Pre-filter cap
    tooltip = "Truncation value for the pre-filtered image pixels; up to ~31.";

    label = new QLabel("Pre-filter cap", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 63);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setPreFilterCap);
    spinBoxPreFilterCap = spinBox;

    layout->addRow(label, spinBox);


    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);


    // Texture threshold
    tooltip = "Texture threshold; areas with no texture (or texture below threshold) are ignored.";

    label = new QLabel("Texture threshold", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 32000);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setTextureThreshold);
    spinBoxTextureThreshold = spinBox;

    layout->addRow(label, spinBox);

    // Uniqueness ratio
    tooltip = "Filter out pixels if there are other close with different disparity, as controlled by this ratio.";

    label = new QLabel("Uniqueness ratio", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 255);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setUniquenessRatio);
    spinBoxUniquenessRatio = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);


    // Scale factor
    tooltip = "";

    label = new QLabel("Scale factor", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 255);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setScaleFactor);
    spinBoxScaleFactor = spinBox;

    layout->addRow(label, spinBox);

    // Smaller block size
    tooltip = "";

    label = new QLabel("Smaller block size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 255);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setSmallerBlockSize);
    spinBoxSmallerBlockSize = spinBox;

    layout->addRow(label, spinBox);

    // Speckle removal technique
    tooltip = "Speckle removal technique.";

    label = new QLabel("Speckle removal technique", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("SPECKLE_REMOVAL_ALGORITHM", cv::stereo::CV_SPECKLE_REMOVAL_ALGORITHM);
    comboBox->setItemData(0, "OpenCV removal algorithm (filterSpeckle()).", Qt::ToolTipRole);
    comboBox->addItem("SPECKLE_REMOVAL_AVG_ALGORITHM", cv::stereo::CV_SPECKLE_REMOVAL_AVG_ALGORITHM);
    comboBox->setItemData(0, "Fill speckles with average disparity of surrounding pixels.", Qt::ToolTipRole);

    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this, comboBox] (int index) {
        method->setSpeckleRemovalTechnique(comboBox->itemData(index).toInt());
    });

    comboBoxSpeckleRemovalTechnique = comboBox;

    layout->addRow(label, comboBox);

    // Binary kernel type
    tooltip = "Binary kernel type.";

    label = new QLabel("Binary kernel type", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    comboBox->addItem("DENSE_CENSUS", cv::stereo::CV_DENSE_CENSUS);
    comboBox->setItemData(0, "Dense census transform.", Qt::ToolTipRole);
    comboBox->addItem("SPARSE_CENSUS", cv::stereo::CV_SPARSE_CENSUS);
    comboBox->setItemData(0, "Sparse census transform.", Qt::ToolTipRole);
    comboBox->addItem("CS_CENSUS", cv::stereo::CV_CS_CENSUS);
    comboBox->setItemData(0, "Center-symmetric census transform.", Qt::ToolTipRole);
    comboBox->addItem("MODIFIED_CS_CENSUS", cv::stereo::CV_MODIFIED_CS_CENSUS);
    comboBox->setItemData(0, "Modified center-symmetric census transform.", Qt::ToolTipRole);
    comboBox->addItem("MODIFIED_CENSUS_TRANSFORM", cv::stereo::CV_MODIFIED_CENSUS_TRANSFORM);
    comboBox->setItemData(0, "Modified census transform.", Qt::ToolTipRole);
    comboBox->addItem("MEAN_VARIATION", cv::stereo::CV_MEAN_VARIATION);
    comboBox->setItemData(0, "Mean-variation transform.", Qt::ToolTipRole);
    comboBox->addItem("STAR_KERNEL", cv::stereo::CV_STAR_KERNEL);
    comboBox->setItemData(0, "STAR kernel descriptor.", Qt::ToolTipRole);

    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this, comboBox] (int index) {
        method->setBinaryKernelType(comboBox->itemData(index).toInt());
    });

    comboBoxBinaryKernelType = comboBox;

    layout->addRow(label, comboBox);

    // Aggregation window size
    tooltip = "";

    label = new QLabel("Aggregation window size", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(0, 255);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), method, &Method::setAggregationWindowSize);
    spinBoxAggregationWindowSize = spinBox;

    layout->addRow(label, spinBox);

/*

    int getSmallerBlockSize () const;
    void setSmallerBlockSize (int value);

    int getScaleFactor () const;
    void setScaleFactor (int value);

    int getSpeckleRemovalTechnique () const;
    void setSpeckleRemovalTechnique (int value);

    int getBinaryKernelType () const;
    void setBinaryKernelType (int value);

    int getAggregationWindowSize () const;
    void setAggregationWindowSize (int value);*/


    //

    //
        /*bm->setPreFilterType((int)storage["preFilterType"]);
    bm->setPreFilterSize((int)storage["preFilterSize"]);
    bm->setPreFilterCap((int)storage["preFilterCap"]);
    bm->setTextureThreshold((int)storage["textureThreshold"]);
    bm->setUniquenessRatio((int)storage["uniquenessRatio"]);
    bm->setSmallerBlockSize((int)storage["blockSize"]);
    bm->setScalleFactor((int)storage["scaleFactor"]);
    bm->setSpekleRemovalTechnique((int)storage["speckleRemovalTechnique"]);
    bm->setUsePrefilter((int)storage["usePrefilter"]);
    bm->setBinaryKernelType((int)storage["binaryKernelType"]);
    bm->setAgregationWindowSize((int)storage["aggregationWindowSize"]);*/



    // Update parameters
    updateParameters();
}

MethodWidget::~MethodWidget ()
{
}

void MethodWidget::updateParameters ()
{
    // Min. disparity
    spinBoxMinDisparity->blockSignals(true);
    spinBoxMinDisparity->setValue(method->getMinDisparity());
    spinBoxMinDisparity->blockSignals(false);

    // Num. disparities
    spinBoxNumDisparities->blockSignals(true);
    spinBoxNumDisparities->setValue(method->getNumDisparities());
    spinBoxNumDisparities->blockSignals(false);

    // Block size
    spinBoxBlockSize->blockSignals(true);
    spinBoxBlockSize->setValue(method->getBlockSize());
    spinBoxBlockSize->blockSignals(false);

    // Speckle window size
    spinBoxSpeckleWindowSize->blockSignals(true);
    spinBoxSpeckleWindowSize->setValue(method->getSpeckleWindowSize());
    spinBoxSpeckleWindowSize->blockSignals(false);

    // Speckle range
    spinBoxSpeckleRange->blockSignals(true);
    spinBoxSpeckleRange->setValue(method->getSpeckleRange());
    spinBoxSpeckleRange->blockSignals(false);

    // Disp12 max diff
    spinBoxDisp12MaxDiff->blockSignals(true);
    spinBoxDisp12MaxDiff->setValue(method->getDisp12MaxDiff());
    spinBoxDisp12MaxDiff->blockSignals(false);


    // Use pre-filter
    checkBoxUsePreFilter->blockSignals(true);
    checkBoxUsePreFilter->setChecked(method->getUsePreFilter());
    checkBoxUsePreFilter->blockSignals(false);

    // Pre-filter type
    comboBoxPreFilterType->blockSignals(true);
    comboBoxPreFilterType->setCurrentIndex(comboBoxPreFilterType->findData(method->getPreFilterType()));
    comboBoxPreFilterType->blockSignals(false);

    // Pre-filter size
    spinBoxPreFilterSize->blockSignals(true);
    spinBoxPreFilterSize->setValue(method->getPreFilterSize());
    spinBoxPreFilterSize->blockSignals(false);

    // Pre-filter cap
    spinBoxPreFilterCap->blockSignals(true);
    spinBoxPreFilterCap->setValue(method->getPreFilterCap());
    spinBoxPreFilterCap->blockSignals(false);


    // Texture threshold
    spinBoxTextureThreshold->blockSignals(true);
    spinBoxTextureThreshold->setValue(method->getTextureThreshold());
    spinBoxTextureThreshold->blockSignals(false);

    // Uniqueness ratio
    spinBoxUniquenessRatio->blockSignals(true);
    spinBoxUniquenessRatio->setValue(method->getUniquenessRatio());
    spinBoxUniquenessRatio->blockSignals(false);


    // Scale factor
    spinBoxScaleFactor->blockSignals(true);
    spinBoxScaleFactor->setValue(method->getScaleFactor());
    spinBoxScaleFactor->blockSignals(false);

    // Smaller block size
    spinBoxSmallerBlockSize->blockSignals(true);
    spinBoxSmallerBlockSize->setValue(method->getSmallerBlockSize());
    spinBoxSmallerBlockSize->blockSignals(false);

    // Speckle removal technique
    comboBoxSpeckleRemovalTechnique->blockSignals(true);
    comboBoxSpeckleRemovalTechnique->setCurrentIndex(comboBoxSpeckleRemovalTechnique->findData(method->getSpeckleRemovalTechnique()));
    comboBoxSpeckleRemovalTechnique->blockSignals(false);

    // Binary kernel type
    comboBoxBinaryKernelType->blockSignals(true);
    comboBoxBinaryKernelType->setCurrentIndex(comboBoxBinaryKernelType->findData(method->getBinaryKernelType()));
    comboBoxBinaryKernelType->blockSignals(false);

    // Aggregation window size
    spinBoxAggregationWindowSize->blockSignals(true);
    spinBoxAggregationWindowSize->setValue(method->getAggregationWindowSize());
    spinBoxAggregationWindowSize->blockSignals(false);
}


} // StereoMethodOpenCvBinaryBm
} // Pipeline
} // StereoToolbox
} // MVL
