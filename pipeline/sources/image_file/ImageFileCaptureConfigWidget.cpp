/*
 * Image File Pair Source: image capture config widget
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
 
#include "ImageFileCaptureConfigWidget.h"
#include "ImageFileCapture.h"


ImageFileCaptureConfigWidget::ImageFileCaptureConfigWidget (ImageFileCapture *c, QWidget *parent)
    : QWidget(parent), capture(c)
{
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QPushButton *button;
    QFrame *line;
    QString tooltip;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Load from files
    QHBoxLayout *buttonBox = new QHBoxLayout();
    layout->addRow(buttonBox);
    
    tooltip = "Load from harddisk.";
    
    button = new QPushButton("File", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, SLOT(loadFile()));
    pushButtonFile = button;

    buttonBox->addWidget(button);

    // Load from URLs
    tooltip = "Load from URL.";
    
    button = new QPushButton("URL", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), this, SLOT(loadUrl()));
    pushButtonUrl = button;

    buttonBox->addWidget(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);
    
    // Image info: filename
    tooltip = "Image file name or URL.";

    label = new QLabel("<b>File/URL:</b>", this);
    label->setToolTip(tooltip);

    textEditFilename = new QTextEdit(this);
    textEditFilename->setReadOnly(true);

    layout->addRow(label, textEditFilename);

    // Image info: resolution
    tooltip = "Image resolution.";

    label = new QLabel("<b>Resolution:</b>", this);
    label->setToolTip(tooltip);

    labelResolution = new QLabel(this);
    labelResolution->setWordWrap(true);

    layout->addRow(label, labelResolution);

    // Image info: channels
    tooltip = "Number of channels.";

    label = new QLabel("<b>Channels:</b>", this);
    label->setToolTip(tooltip);

    labelChannels = new QLabel(this);
    labelChannels->setWordWrap(true);

    layout->addRow(label, labelChannels);

    // URL dialog
    dialogUrl = new UrlDialog(this);

    // Signals
    connect(capture, SIGNAL(imageReady()), this, SLOT(updateImageInformation()));
}

ImageFileCaptureConfigWidget::~ImageFileCaptureConfigWidget ()
{
}

void ImageFileCaptureConfigWidget::updateImageInformation ()
{
    // Display image information
    textEditFilename->setText(QString("%1").arg(capture->getImageFilename()));
    labelResolution->setText(QString("%1x%2").arg(capture->getImageWidth()).arg(capture->getImageHeight()));
    labelChannels->setText(QString("%1").arg(capture->getImageChannels()));
}

void ImageFileCaptureConfigWidget::loadFile ()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load image", QString(), "Images (*.png *.jpg *.pgm *.ppm *.tif *.bmp)");
    if (!filename.isEmpty()) {
        capture->setImageFileOrUrl(filename, false);
    }
}


void ImageFileCaptureConfigWidget::loadUrl ()
{
    // Run the dialog
    if (dialogUrl->exec() == QDialog::Accepted) {
        // Set URL
        capture->setImageFileOrUrl(dialogUrl->getUrl(), true);
    }
}



// *********************************************************************
// *                            URL dialog                             *
// *********************************************************************
UrlDialog::UrlDialog (QWidget *parent)
    : QDialog(parent)
{
    QFormLayout *layout = new QFormLayout(this);

    textEditUrl = new QTextEdit(this);
    layout->addRow("Image URL", textEditUrl);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    layout->addRow(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

UrlDialog::~UrlDialog ()
{
}

QString UrlDialog::getUrl() const
{
    return textEditUrl->toPlainText();
}
