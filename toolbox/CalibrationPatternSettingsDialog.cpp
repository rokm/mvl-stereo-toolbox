/*
 * MVL Stereo Toolbox: calibration GUI: calibration pattern settings dialog
 * Copyright (C) 2013 Rok Mandeljc
 *
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

#include "CalibrationPatternSettingsDialog.h"

#include "StereoCalibration.h"


CalibrationPatternSettingsDialog::CalibrationPatternSettingsDialog (QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Calibration pattern");

    // Layout
    QFormLayout *layout = new QFormLayout(this);
    setLayout(layout);

    QLabel *label;
    QFrame *separator;
    QDialogButtonBox *buttonBox;

    // Pattern width
    label = new QLabel("Pattern width", this);
    label->setToolTip("Pattern width:\n"
                      " - chessboard: number of inside corners in horizontal direction\n"
                      " - circle grid: number of circles in horizontal direction\n"
                      " - asymmetric circle grid: number of circles in first column (*vertical* direction!)");
    
    spinBoxPatternWidth = new QSpinBox(this);
    spinBoxPatternWidth->setRange(1, 1000);
    spinBoxPatternWidth->setValue(19);
    layout->addRow(label, spinBoxPatternWidth);

    // Pattern height
    label = new QLabel("Pattern height", this);
    label->setToolTip("Pattern width:\n"
                      " - chessboard: number of inside corners in vertical direction\n"
                      " - circle grid: number of circles in vertical direction\n"
                      " - asymmetric circle grid: total number of circles in first *two* rows (*horizontal* direction!)");
    
    spinBoxPatternHeight = new QSpinBox(this);
    spinBoxPatternHeight->setRange(1, 1000);
    spinBoxPatternHeight->setValue(12);
    layout->addRow(label, spinBoxPatternHeight);

    // Element size
    label = new QLabel("Element size", this);
    label->setToolTip("Size of pattern elements:\n"
                      " - chessboard: square size (distance between two corners)"
                      " - circle grid: distance between circle centers"
                      " - asymmetric circle grid: distance between circle centers");

    spinBoxElementSize = new QDoubleSpinBox(this);
    spinBoxElementSize->setRange(1.0, 1000.0);
    spinBoxElementSize->setValue(25.0);
    spinBoxElementSize->setSuffix(" mm");
    layout->addRow(label, spinBoxElementSize);

    // Pattern type
    label = new QLabel("Pattern type", this);
    label->setToolTip("Calibration pattern type");

    comboBoxPatternType = new QComboBox(this);
    comboBoxPatternType->addItem("Chessboard", CalibrationPattern::Chessboard);
    comboBoxPatternType->addItem("Circle grid", CalibrationPattern::Circles);
    comboBoxPatternType->addItem("Asymmetric circle grid", CalibrationPattern::AsymmetricCircles);
    comboBoxPatternType->setCurrentIndex(0);
    layout->addRow(label, comboBoxPatternType);

    // Separator
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addRow(separator);

    // Max image scale level
    label = new QLabel("Image scale levels", this);
    label->setToolTip("Maximum image scale level. If pattern is not found at original image size,\n"
                      "image is upsampled and search is repeated. The image scale is 1.0 + level*scaleIncrement,\n"
                      "where level goes from 0 to imageScaleLevels. Set this variable to 0 to disable\n"
                      "multi-scale search.");

    spinBoxScaleLevels = new QSpinBox(this);
    spinBoxScaleLevels->setRange(0, 100);
    spinBoxScaleLevels->setValue(0);
    layout->addRow(label, spinBoxScaleLevels);

    // Max image scale level
    label = new QLabel("Scale increment", this);
    label->setToolTip("Scale increment for multi-scale pattern search. For details, see description of \n"
                      "image scale levels.");

    spinBoxScaleIncrement = new QDoubleSpinBox(this);
    spinBoxScaleIncrement->setRange(0.0, 2.0);
    spinBoxScaleIncrement->setSingleStep(0.05);
    spinBoxScaleIncrement->setValue(0.25);
    layout->addRow(label, spinBoxScaleIncrement);

    // Separator
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addRow(separator);

    // Show results
    checkBoxShowResult = new QCheckBox("Show results", this);
    checkBoxShowResult->setToolTip("Show detection results and allow user to discard them.");
    checkBoxShowResult->setChecked(true);
    layout->addRow(checkBoxShowResult);

    // Separator
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addRow(separator);

    // Button box
    buttonBox = new QDialogButtonBox(this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addRow(buttonBox);
    
    buttonBox->addButton(QDialogButtonBox::Ok);
    buttonBox->addButton(QDialogButtonBox::Cancel);
}
    
CalibrationPatternSettingsDialog::~CalibrationPatternSettingsDialog ()
{
}


CalibrationPattern CalibrationPatternSettingsDialog::getPattern () const
{
    return CalibrationPattern(
        spinBoxPatternWidth->value(),
        spinBoxPatternHeight->value(),
        spinBoxElementSize->value(),
        (CalibrationPattern::PatternType)comboBoxPatternType->itemData(comboBoxPatternType->currentIndex()).toInt(),
        spinBoxScaleLevels->value(),
        spinBoxScaleIncrement->value()
    );
}

bool CalibrationPatternSettingsDialog::getShowResult () const
{
    return checkBoxShowResult->checkState() == Qt::Checked;
}
