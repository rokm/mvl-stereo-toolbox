/*
 * OpenCV Camera Image Pair Source: config widget
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

#include "source_widget.h"
#include "source.h"
#include "camera.h"

using namespace SourceOpenCvCam;


SourceWidget::SourceWidget (Source *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    configLeftDevice = NULL;
    configRightDevice = NULL;

    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QFrame *line;
    QPushButton *button;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>OpenCV camera source</u></b>", this);
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
    

    // Rescan
    tooltip = "Rescan connected cameras.";
    
    button = new QPushButton("Rescan");
    button->setToolTip(tooltip);
    connect(button, &QPushButton::clicked, source, &Source::refreshCameraList);

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Devices
    boxDevices = new QHBoxLayout();
    layout->addRow(boxDevices);

    boxDevices->addWidget(createDeviceFrame(true)); // Left device frame
    boxDevices->addWidget(createDeviceFrame(false)); // Right device frame

    connect(source, &Source::leftCameraChanged, this, [this] {
        updateCamera(configLeftDevice, frameLeftDevice, source->getLeftCamera());
    });
    connect(source, &Source::rightCameraChanged, this, [this] {
        updateCamera(configRightDevice, frameRightDevice, source->getRightCamera());
    });
}

SourceWidget::~SourceWidget ()
{
}



QWidget *SourceWidget::createDeviceFrame (bool left)
{
    QFrame *deviceFrame, *frame;
    QLabel *label;
    QComboBox *comboBox;
    QString tooltip;

    QFormLayout *layout;
   
    // Camera frame
    deviceFrame = new QFrame(this);
    deviceFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    layout = new QFormLayout(deviceFrame);

    // Label
    label = new QLabel(left ? "<b>Left device</b>" : "<b>Right device</b>", deviceFrame);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(label);

    // Combo box
    tooltip = left ? "Left OpenCV camera device." : "Right OpenCV camera device.";
    
    comboBox = new QComboBox(deviceFrame);
    comboBox->setModel(source);
    comboBox->setToolTip(tooltip);
    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &SourceWidget::deviceSelected);
    if (left) {
        comboBoxLeftDevice = comboBox;
    } else {
        comboBoxRightDevice = comboBox;
    }
    layout->addRow(comboBox);

    // Camera config frame
    frame = new QFrame(deviceFrame);
    QVBoxLayout *frameLayout = new QVBoxLayout(frame);
    frame->setContentsMargins(0, 0, 0, 0);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    layout->addRow(frame);
    if (left) {
        frameLeftDevice = frame;
    } else {
        frameRightDevice = frame;
    }

    return deviceFrame;
}


// *********************************************************************
// *                          Device selection                         *
// *********************************************************************
void SourceWidget::deviceSelected (int index)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(QObject::sender());
    QVariant c = comboBox->itemData(index);
    int device = c.isValid() ? c.toInt() : -1;
    
    if (comboBox == comboBoxLeftDevice) {
        source->setLeftCamera(device);
    } else if (comboBox == comboBoxRightDevice) {
        source->setRightCamera(device);
    }
}

void SourceWidget::updateCamera (QWidget *&deviceConfig, QFrame *&deviceFrame, Camera *newDevice)
{
    // Remove config widget for old device
    if (deviceConfig) {
        deviceFrame->layout()->removeWidget(deviceConfig);
        deviceConfig->deleteLater();
        deviceConfig = NULL;
    }

    // Get new device's config widget
    if (newDevice) {
        deviceConfig = newDevice->createConfigWidget(this);
        deviceFrame->layout()->addWidget(deviceConfig);
    }
}
