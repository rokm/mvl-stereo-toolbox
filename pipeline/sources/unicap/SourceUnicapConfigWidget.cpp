/*
 * Unicap Image Pair Source: config widget
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

#include "SourceUnicapConfigWidget.h"
#include "SourceUnicap.h"

#include "CameraUnicap.h"
#include "CameraListModel.h"


SourceUnicapConfigWidget::SourceUnicapConfigWidget (SourceUnicap *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    configLeftDevice = NULL;
    configRightDevice = NULL;

    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QFrame *line;
    QPushButton *button;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>Unicap source</u></b>", this);
    label->setAlignment(Qt::AlignHCenter);
    
    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Rescan
    tooltip = "Rescan Unicap devices.";
    
    button = new QPushButton("Rescan");
    button->setToolTip(tooltip);
    connect(button, SIGNAL(clicked()), source, SLOT(scanForDevices()));

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

SourceUnicapConfigWidget::~SourceUnicapConfigWidget ()
{
}



QWidget *SourceUnicapConfigWidget::createDeviceFrame (bool left)
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
    deviceFrame->setLayout(layout);

    // Label
    label = new QLabel(left ? "<b>Left device</b>" : "<b>Right device</b>", deviceFrame);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(label);

    // Combo box
    tooltip = left ? "Left Unicap device." : "Right Unicap device.";
    
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
void SourceUnicapConfigWidget::deviceSelected (int index)
{
    if (QObject::sender() == comboBoxLeftDevice) {
        deviceSelected(configLeftDevice, frameLeftDevice, comboBoxLeftDevice, index);
    } else if (QObject::sender() == comboBoxRightDevice) {
        deviceSelected(configRightDevice, frameRightDevice, comboBoxRightDevice, index);
    } else {
        qFatal("Invalid sender!");
    }
}

void SourceUnicapConfigWidget::deviceSelected (QWidget *&deviceConfig, QFrame *&deviceFrame, QComboBox *&deviceComboBox, int index)
{
    // Remove config widget for old device
    if (deviceConfig) {
        deviceFrame->layout()->removeWidget(deviceConfig);
        deviceConfig->deleteLater();
        deviceConfig = NULL;
    }
    
    // Set new device
    CameraUnicap *newDevice;
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
