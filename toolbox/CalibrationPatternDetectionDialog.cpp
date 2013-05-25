/*
 * MVL Stereo Toolbox: calibration GUI: calibration pattern detection dialog
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

#include "CalibrationPatternDetectionDialog.h"

#include "CalibrationPatternDisplayWidget.h"


CalibrationPatternDetectionDialog::CalibrationPatternDetectionDialog (QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Pattern detection results");
    resize(800, 600);

    QFrame *separator;
    QDialogButtonBox *buttonBox;

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Image
    displayImage = new CalibrationPatternDisplayWidget("Pattern image", this);
    displayImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayImage);

    // Separator
    separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(separator);
    
    // Button box
    buttonBox = new QDialogButtonBox(this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox);

    pushButtonAccept = new QPushButton("Accept", this);
    buttonBox->addButton(pushButtonAccept, QDialogButtonBox::AcceptRole);

    pushButtonDiscard = new QPushButton("Discard", this);
    buttonBox->addButton(pushButtonDiscard, QDialogButtonBox::RejectRole);
}

CalibrationPatternDetectionDialog::~CalibrationPatternDetectionDialog ()
{
}

void CalibrationPatternDetectionDialog::setImage (const cv::Mat &img, bool found, const std::vector<cv::Point2f> &points, const cv::Size &patternSize)
{
    pushButtonAccept->setEnabled(found); // Disable "accept" if pattern has not been found
    displayImage->setImage(img, found, points, patternSize);
}
