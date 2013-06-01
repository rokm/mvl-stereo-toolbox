/*
 * File Image Source: config widget
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

#include "ImageSourceFileConfigWidget.h"
#include "ImageSourceFile.h"


ImageSourceFileConfigWidget::ImageSourceFileConfigWidget (ImageSourceFile *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QPushButton *button;
    QFrame *line;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>File source</u><b>", this);
    label->setAlignment(Qt::AlignHCenter);

    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Load
    tooltip = "Load new pair of images.";
    
    button = new QPushButton("Load images", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, SLOT(loadImages()));
    buttonLoadImages = button;

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Left image
    tooltip = "Left image name.";

    label = new QLabel("<b>Left image: </b>", this);
    label->setToolTip(tooltip);
    labelFilenameLeft = label;

    layout->addRow(label);

    // Right image
    tooltip = "Right image name.";

    label = new QLabel("<b>Right image: </b>", this);
    label->setToolTip(tooltip);
    labelFilenameRight = label;

    layout->addRow(label);
}

ImageSourceFileConfigWidget::~ImageSourceFileConfigWidget ()
{
}

void ImageSourceFileConfigWidget::loadImages ()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Load left and right image", QString(), "Images (*.png *.jpg *.pgm *.ppm *.tif *.bmp)");

    // Take first two images
    if (filenames.size() >= 2) {
        // Load image pair
        source->loadImagePair(filenames[0], filenames[1]);

        // Display image information
        QFileInfo fileLeft(source->getLeftFilename());
        QFileInfo fileRight(source->getRightFilename());

        labelFilenameLeft->setText(QString("<b>Left image: </b> %1, %2x%3, %4 ch.").arg(fileLeft.fileName()).arg(source->getLeftWidth()).arg(source->getLeftHeight()).arg(source->getLeftChannels()));
        labelFilenameRight->setText(QString("<b>Right image: </b> %1, %2x%3, %4 ch.").arg(fileRight.fileName()).arg(source->getRightWidth()).arg(source->getRightHeight()).arg(source->getRightChannels()));
    }
}
