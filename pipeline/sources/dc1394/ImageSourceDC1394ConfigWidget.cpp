/*
 * DC1394 Image Source: config widget
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

#include "ImageSourceDC1394ConfigWidget.h"
#include "ImageSourceDC1394.h"

#include "CameraDC1394.h"
#include "CameraListModel.h"


ImageSourceDC1394ConfigWidget::ImageSourceDC1394ConfigWidget (ImageSourceDC1394 *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    configLeftDevice = NULL;
    configRightDevice = NULL;

    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QFrame *line, *frame;
    QPushButton *button;
    QComboBox *comboBox;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>DC1394 source</u></b>", this);
    label->setAlignment(Qt::AlignHCenter);
    
    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Rescan
    tooltip = "Rescan bus for DC1394 devices.";
    
    button = new QPushButton("Rescan");
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), source, SLOT(scanBus()));

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
}

ImageSourceDC1394ConfigWidget::~ImageSourceDC1394ConfigWidget ()
{
}



QWidget *ImageSourceDC1394ConfigWidget::createDeviceFrame (bool left)
{
    QFrame *deviceFrame, *frame;
    QLabel *label;
    QComboBox *comboBox;
    QPushButton *button;
    QFrame *line;
    QString tooltip;

    QFormLayout *layout;
   
    // Camera frame
    deviceFrame = new QFrame(this);
    deviceFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    layout = new QFormLayout(deviceFrame);
    deviceFrame->setLayout(layout);

    // Label
    label = new QLabel(left ? "<b>Left device</b>" : "<b>Right device</b>", deviceFrame);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(label);

    // Combo box
    tooltip = left ? "Left DC1394 device." : "Right DC1394 device.";
    
    comboBox = new QComboBox(deviceFrame);
    comboBox->setModel(source->getCameraListModel());
    comboBox->setToolTip(tooltip);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(deviceSelected(int)));
    if (left) {
        comboBoxLeftDevice = comboBox;
    } else {
        comboBoxRightDevice = comboBox;
    }
    layout->addRow(comboBox);

    // Capture
    tooltip = "Start/stop capture.";
    
    button = new QPushButton("Capture", deviceFrame);
    button->setToolTip(tooltip);
    button->setCheckable(true);
    connect(button, SIGNAL(toggled(bool)), this, SLOT(startStopCapture(bool)));
    if (left) {
        pushButtonCaptureLeftDevice = button;
    } else {
        pushButtonCaptureRightDevice = button;
    }

    layout->addRow(button);

    // Camera config frame
    frame = new QFrame(deviceFrame);
    frame->setLayout(new QVBoxLayout(frame));
    frame->setContentsMargins(0, 0, 0, 0);
    frame->layout()->setContentsMargins(0, 0, 0, 0);
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
void ImageSourceDC1394ConfigWidget::deviceSelected (int index)
{
    QWidget **deviceConfig;
    QFrame **frame;
    QComboBox **comboBox;

    if (QObject::sender() == comboBoxLeftDevice) {
        deviceSelected(configLeftDevice, frameLeftDevice, comboBoxLeftDevice, index);
    } else if (QObject::sender() == comboBoxRightDevice) {
        deviceSelected(configRightDevice, frameRightDevice, comboBoxRightDevice, index);
    } else {
        qFatal("Invalid sender!");
    }
}

void ImageSourceDC1394ConfigWidget::deviceSelected (QWidget *&deviceConfig, QFrame *&deviceFrame, QComboBox *&deviceComboBox, int index)
{
    
    // Remove config widget for old device
    if (deviceConfig) {
        deviceFrame->layout()->removeWidget(deviceConfig);
        deviceConfig->deleteLater();
        deviceConfig = NULL;
    }
    
    // Set new device
    CameraDC1394 *newDevice;
    QVariant c = deviceComboBox->itemData(index);
    
    if (deviceComboBox == comboBoxLeftDevice) {
        source->setLeftCamera(c.isValid() ? c.toInt() : -1);
        newDevice = source->getLeftCamera();
    } else {
        source->setRightCamera(c.isValid() ? c.toInt() : -1);
        newDevice = source->getRightCamera();
    }

    // Get new device's config widget
    if (newDevice) {
        deviceConfig = newDevice->createConfigWidget(this);
        deviceFrame->layout()->addWidget(deviceConfig);
    }
}


// *********************************************************************
// *                     Device start/stop capture                     *
// *********************************************************************
void ImageSourceDC1394ConfigWidget::startStopCapture (bool start)
{
    if (QObject::sender() == pushButtonCaptureLeftDevice) {
        startStopCapture(source->getLeftCamera(), start);        
    } else {
        startStopCapture(source->getRightCamera(), start);
    }
}

void ImageSourceDC1394ConfigWidget::startStopCapture (CameraDC1394 *device, bool start)
{
    if (!device) {
        return;
    }
    
    if (start) {
        device->startCapture();
    } else {
        device->stopCapture();
    }
}
