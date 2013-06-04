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
    QSpinBox *spinBox;
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

    // Load from files
    tooltip = "Load new pair of images from harddisk.";
    
    button = new QPushButton("Load from files", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, SLOT(loadFiles()));
    pushButtonLoadFiles = button;

    layout->addRow(button);

    // Load from URLs
    tooltip = "Load new pair of images from URLs.";
    
    button = new QPushButton("Load from URLs", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, SLOT(loadUrls()));
    pushButtonLoadUrls = button;

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

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
    
    // Left image
    tooltip = "Left image name.";

    label = new QLabel("<b>Left image: </b>", this);
    label->setToolTip(tooltip);
    label->setWordWrap(true);
    labelFilenameLeft = label;

    layout->addRow(label);

    // Right image
    tooltip = "Right image name.";

    label = new QLabel("<b>Right image: </b>", this);
    label->setToolTip(tooltip);
    label->setWordWrap(true);
    labelFilenameRight = label;

    layout->addRow(label);

    // URL dialog
    dialogUrl = new UrlDialog(this);

    // Signals
    connect(source, SIGNAL(imagesChanged()), this, SLOT(updateImageInformation()));
}

ImageSourceFileConfigWidget::~ImageSourceFileConfigWidget ()
{
}

void ImageSourceFileConfigWidget::updateImageInformation ()
{
    // Display image information
    labelFilenameLeft->setText(QString("<b>Left image: </b> %1, <b>resolution:</b> %2x%3, <b>channels:</b> %4").arg(source->getLeftFilename()).arg(source->getLeftWidth()).arg(source->getLeftHeight()).arg(source->getLeftChannels()));
    labelFilenameRight->setText(QString("<b>Right image: </b> %1, <b>resolution:</b> %2x%3, <b>channels:</b> %4").arg(source->getRightFilename()).arg(source->getRightWidth()).arg(source->getRightHeight()).arg(source->getRightChannels()));
}

void ImageSourceFileConfigWidget::loadFiles ()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Load left and right image", QString(), "Images (*.png *.jpg *.pgm *.ppm *.tif *.bmp)");

    // Take first two images
    if (filenames.size() >= 2) {
        // Load image pair
        source->loadImagePair(filenames[0], filenames[1], false);
    }
}


void ImageSourceFileConfigWidget::loadUrls ()
{
    // Run the dialog
    if (dialogUrl->exec() == QDialog::Accepted) {
        // Load image pair
        source->loadImagePair(dialogUrl->getUrlLeft(), dialogUrl->getUrlRight(), true);
    }
}



// *********************************************************************
// *                            URL dialog                             *
// *********************************************************************
UrlDialog::UrlDialog (QWidget *parent)
    : QDialog(parent)
{
    QFormLayout *layout = new QFormLayout(this);

    textEditUrl1 = new QTextEdit(this);
    layout->addRow("Left image URL", textEditUrl1);

    textEditUrl2 = new QTextEdit(this);
    layout->addRow("Right image URL", textEditUrl2);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    layout->addRow(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

UrlDialog::~UrlDialog ()
{
}

QString UrlDialog::getUrlLeft () const
{
    return textEditUrl1->toPlainText();
}

QString UrlDialog::getUrlRight () const
{
    return textEditUrl2->toPlainText();
}
