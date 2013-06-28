/*
 * DC1394 Image Pair Source: config widget
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

#include "SourceDC1394ConfigWidget.h"
#include "SourceDC1394.h"

#include "CameraDC1394.h"
#include "CameraListModel.h"


SourceDC1394ConfigWidget::SourceDC1394ConfigWidget (SourceDC1394 *s, QWidget *parent)
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

    connect(source, SIGNAL(leftCameraChanged()), this, SLOT(updateLeftCamera()));
    connect(source, SIGNAL(rightCameraChanged()), this, SLOT(updateRightCamera()));
}

SourceDC1394ConfigWidget::~SourceDC1394ConfigWidget ()
{
}



QWidget *SourceDC1394ConfigWidget::createDeviceFrame (bool left)
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
void SourceDC1394ConfigWidget::deviceSelected (int index)
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


void SourceDC1394ConfigWidget::updateLeftCamera ()
{
    updateCamera(configLeftDevice, frameLeftDevice, source->getLeftCamera());
}

void SourceDC1394ConfigWidget::updateRightCamera ()
{
    updateCamera(configRightDevice, frameRightDevice, source->getRightCamera());
}

void SourceDC1394ConfigWidget::updateCamera (QWidget *&deviceConfig, QFrame *&deviceFrame, CameraDC1394 *newDevice)
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
