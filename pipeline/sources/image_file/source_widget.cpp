/*
 * Image File Source: source widget
 * Copyright (C) 2013-2015 Rok Mandeljc
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

#include "source_widget.h"
#include "source.h"
#include "image_file.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceImageFile {


SourceWidget::SourceWidget (Source *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QPushButton *button;
    QSpinBox *spinBox;
    QFrame *line;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>Image file source</u><b>", this);
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


    // Load from files
    tooltip = "Load new pair of images from harddisk.";

    button = new QPushButton("Load image pair", this);
    button->setToolTip(tooltip);
    connect(button, &QPushButton::clicked, this, [this] {
        // Get filename
        QStringList filenames = QFileDialog::getOpenFileNames(this, "Load left and right image", QFileInfo(source->getLeftImageFile()->getImageFilename()).filePath(), "Images (*.png *.jpg *.pgm *.ppm *.tif *.bmp)");

        // Take first two images
        if (filenames.size() >= 2) {
            // Load image pair
            source->loadImagePair(filenames[0], filenames[1], false);
        }
    });

    layout->addRow(button);

    // Periodic refresh
    tooltip = "Enable/disable perodic refresh";

    button = new QPushButton("Periodic refresh", this);
    button->setToolTip(tooltip);
    button->setCheckable(true);
    connect(button, &QPushButton::toggled, source, &Source::setPeriodicRefreshState);
    connect(source, &Source::periodicRefreshStateChanged, button, &QPushButton::setChecked);

    layout->addRow(button);

    // Refresh period
    tooltip = "Refresh period for periodic refresh";

    label = new QLabel("Refresh period", this);
    label->setToolTip(tooltip);

    spinBox = new QSpinBox(this);
    spinBox->setKeyboardTracking(false);
    spinBox->setRange(1, 9999);
    spinBox->setSingleStep(1000);
    spinBox->setValue(1000);
    spinBox->setSuffix(" ms"); //
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), source, &Source::setRefreshPeriod);
    connect(source, &Source::refreshPeriodChanged, spinBox, &QSpinBox::setValue);

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

SourceWidget::~SourceWidget ()
{
}


QWidget *SourceWidget::createImageFrame (bool left)
{
    QFrame *imageFrame;
    QLabel *label;
    QString tooltip;

    QFormLayout *layout;

    // Image frame
    imageFrame = new QFrame(this);
    imageFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    layout = new QFormLayout(imageFrame);

    // Label
    label = new QLabel(left ? "<b>Left image</b>" : "<b>Right image</b>", imageFrame);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(label);

    // Widget
    if (left) {
        layout->addRow(source->getLeftImageFile()->createConfigWidget(this));
    } else {
        layout->addRow(source->getRightImageFile()->createConfigWidget(this));
    }

    return imageFrame;
}


} // SourceImageFile
} // Pipeline
} // StereoToolbox
} // MVL
