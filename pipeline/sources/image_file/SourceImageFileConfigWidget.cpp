/*
 * Image File Pair Source: config widget
 * Copyright (C) 2013 Rok Mandeljc
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

#include "SourceImageFileConfigWidget.h"
#include "SourceImageFile.h"
#include "ImageFileCapture.h"


SourceImageFileConfigWidget::SourceImageFileConfigWidget (SourceImageFile *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QPushButton *button;
    QSpinBox *spinBox;
    QFrame *line;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>Image file source</u><b>", this);
    label->setAlignment(Qt::AlignHCenter);

    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Load from files
    tooltip = "Load new pair of images from harddisk.";
    
    button = new QPushButton("Load image pair", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, SLOT(loadImagePair()));
    pushButtonLoadPair = button;

    layout->addRow(button);

    // Periodic refresh
    tooltip = "Enable/disable perodic refresh";
    
    button = new QPushButton("Periodic refresh", this);
    button->setToolTip(tooltip);
    button->setCheckable(true);
    connect(button, SIGNAL(toggled(bool)), source, SLOT(setPeriodicRefreshState(bool)));
    connect(source, SIGNAL(periodicRefreshStateChanged(bool)), button, SLOT(setChecked(bool)));
    pushButtonPeriodicRefresh = button;

    layout->addRow(button);

    // Refresh period
    tooltip = "Refresh period for periodic refresh";

    label = new QLabel("Refresh period", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, INT_MAX);
    spinBox->setSingleStep(1000);
    spinBox->setValue(1000);
    spinBox->setSuffix(" ms"); // 
    connect(spinBox, SIGNAL(valueChanged(int)), source, SLOT(setRefreshPeriod(int)));
    connect(source, SIGNAL(refreshPeriodChanged(int)), spinBox, SLOT(setValue(int)));
    spinBoxRefreshPeriod = spinBox;

    layout->addRow(label, spinBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Image sources
    QHBoxLayout *boxImages = new QHBoxLayout();
    layout->addRow(boxImages);

    boxImages->addWidget(createImageFrame(true)); // Left device frame
    boxImages->addWidget(createImageFrame(false)); // Right device frame
}

SourceImageFileConfigWidget::~SourceImageFileConfigWidget ()
{
}


QWidget *SourceImageFileConfigWidget::createImageFrame (bool left)
{
    QFrame *imageFrame;
    QLabel *label;
    QString tooltip;

    QFormLayout *layout;
   
    // Image frame
    imageFrame = new QFrame(this);
    imageFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    layout = new QFormLayout(imageFrame);
    imageFrame->setLayout(layout);

    // Label
    label = new QLabel(left ? "<b>Left image</b>" : "<b>Right image</b>", imageFrame);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(label);

    // Widget
    if (left) {
        layout->addRow(source->getLeftImageCapture()->createConfigWidget(this));
    } else {
        layout->addRow(source->getRightImageCapture()->createConfigWidget(this));
    }
    
    return imageFrame;
}


void SourceImageFileConfigWidget::loadImagePair ()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Load left and right image", QString(), "Images (*.png *.jpg *.pgm *.ppm *.tif *.bmp)");

    // Take first two images
    if (filenames.size() >= 2) {
        // Load image pair
        source->loadImagePair(filenames[0], filenames[1], false);
    }
}
