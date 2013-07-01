/*
 * Unicap Camera: config widget
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

#include "CameraUnicapConfigWidget.h"
#include "CameraUnicap.h"

#include "PropertyWidget.h"


CameraUnicapConfigWidget::CameraUnicapConfigWidget (CameraUnicap *c, QWidget *parent)
    : QWidget(parent), camera(c)
{
    QFormLayout *layout = new QFormLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label;
    QComboBox *comboBox;
    QPushButton *button;
    QFrame *line;

    QString tooltip;

    connect(camera, SIGNAL(formatChanged()), this, SLOT(updateFormat()));
    connect(camera, SIGNAL(captureStarted()), this, SLOT(updateCameraState()));
    connect(camera, SIGNAL(captureFinished()), this, SLOT(updateCameraState()));

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Vendor
    tooltip = "Device vendor.";
    
    label = new QLabel("<b>Vendor: </b>" + camera->getDeviceVendor(), this);
    label->setToolTip(tooltip);

    layout->addRow(label);

    // Model
    tooltip = "Device model.";
    
    label = new QLabel("<b>Model: </b>" + camera->getDeviceModel(), this);
    label->setToolTip(tooltip);

    layout->addRow(label);

    // Communication plugin
    tooltip = "Device communication plugin.";
    
    label = new QLabel("<b>Plugin: </b>" + camera->getDevicePluginName(), this);
    label->setToolTip(tooltip);

    layout->addRow(label);

    // Device
    tooltip = "Device file name.";
    
    label = new QLabel("<b>Device: </b>" + camera->getDeviceFileName(), this);
    label->setToolTip(tooltip);

    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    
    // Capture
    tooltip = "Start/stop capture.";
    
    button = new QPushButton("Capture", this);
    button->setToolTip(tooltip);
    button->setCheckable(true);
    connect(button, SIGNAL(toggled(bool)), this, SLOT(captureButtonToggled(bool)));
    pushButtonCapture = button;

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Format
    tooltip = "Camera resolution and color mode.";
    
    label = new QLabel("Format", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(comboBoxFormatActivated(int)));
    comboBoxFormat = comboBox;
    
    layout->addRow(label, comboBox);

    foreach (const unicap_format_t &format, camera->getSupportedFormats()) {
        QString entry(format.identifier);
        comboBoxFormat->addItem(entry, entry);
    }

    // Size 1
    tooltip = "Image size";

    label = new QLabel("Size", this);
    label->setToolTip(tooltip);
    labelSize1 = label;

    widgetSize = new SizeWidget(this);
    connect(widgetSize, SIGNAL(sizeChanged(unicap_rect_t)), this, SLOT(sizeWidgetChanged(unicap_rect_t)));
    
    layout->addRow(label, widgetSize);

    // Size 2
    tooltip = "Image size";

    label = new QLabel("Size", this);
    label->setToolTip(tooltip);
    labelSize2 = label;

    comboBox = new QComboBox(this);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(comboBoxSizeActivated(int)));
    comboBoxSize = comboBox;
    
    layout->addRow(label, comboBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Property widgets
    addPropertyWidgets();

    // Update format
    updateFormat();
}

CameraUnicapConfigWidget::~CameraUnicapConfigWidget ()
{
}

void CameraUnicapConfigWidget::addPropertyWidgets ()
{
    const QVector<unicap_property_t> &properties = camera->getSupportedProperties();

    QLabel *label;
    PropertyWidget *widget;

    foreach (const unicap_property_t &property, properties) {
        label = new QLabel(property.identifier, this);
        widget = new PropertyWidget(camera, property, this);

        qobject_cast<QFormLayout *>(layout())->addRow(label, widget);
    }
}


void CameraUnicapConfigWidget::comboBoxFormatActivated (int index)
{
    camera->setFormat(camera->getSupportedFormats()[index]);
}

void CameraUnicapConfigWidget::comboBoxSizeActivated (int index)
{
    camera->setSize(format.sizes[index]);
}

void CameraUnicapConfigWidget::sizeWidgetChanged (unicap_rect_t size)
{
    camera->setSize(size);
}


void CameraUnicapConfigWidget::updateFormat ()
{
    bool oldState;
    format = camera->getFormat();

    // Format
    oldState = comboBoxFormat->blockSignals(true);
    comboBoxFormat->setCurrentIndex(comboBoxFormat->findData(format.identifier));
    comboBoxFormat->blockSignals(oldState);

    // Size
    widgetSize->setValidSizeRange(format.min_size, format.max_size, format.h_stepping, format.v_stepping);

    if (format.size_count) {
        comboBoxSize->clear();
        for (int i = 0; i < format.size_count; i++) {
            const unicap_rect_t &size = format.sizes[i];
            comboBoxSize->addItem(QString("+%1+%2 %3x%4").arg(size.x).arg(size.y).arg(size.width).arg(size.height));
        }

        // Hide size widget, make sure combo box is shown
        labelSize1->hide();
        widgetSize->hide();

        labelSize2->show();
        comboBoxSize->show();
    } else {
        // Hide combo box, make sure size widget is shown
        labelSize1->show();
        widgetSize->show();

        labelSize2->hide();
        comboBoxSize->hide();
    }
    
    updateSize();
}

void CameraUnicapConfigWidget::updateSize ()
{
    bool oldState;
    format = camera->getFormat();

    // Size in widget
    widgetSize->setSize(format.size);

    int idx = -1;
    for (int i = 0; i < format.size_count; i++) {
        if (!memcmp(&format.size, &format.sizes[i], sizeof(format.size))) {
            idx = i;
            break;
        }
    }

    // Select size
    oldState = comboBoxSize->blockSignals(true);
    comboBoxSize->setCurrentIndex(idx);
    comboBoxSize->blockSignals(oldState);
}


void CameraUnicapConfigWidget::captureButtonToggled (bool start)
{
    if (start) {
        camera->startCapture();
    } else {
        camera->stopCapture();
    }
}


void CameraUnicapConfigWidget::updateCameraState ()
{
    bool oldState;
    oldState = pushButtonCapture->blockSignals(true);
    pushButtonCapture->setChecked(camera->getCaptureState());
    pushButtonCapture->blockSignals(oldState);
}


// *********************************************************************
//
// *********************************************************************
SizeWidget::SizeWidget (QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label;
    
    // Offset X, Y    
    label = new QLabel("X:", this);
    label->setToolTip("Horizontal offset.");
    layout->addWidget(label, 0, 0);

    spinBoxOffsetX = new QSpinBox(this);
    spinBoxOffsetX->setKeyboardTracking(false);
    spinBoxOffsetX->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(spinBoxOffsetX, 0, 1);

    label = new QLabel("Y:", this);
    label->setToolTip("Vertical offset.");
    layout->addWidget(label, 0, 2);

    spinBoxOffsetY = new QSpinBox(this);
    spinBoxOffsetY->setKeyboardTracking(false);
    spinBoxOffsetY->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(spinBoxOffsetY, 0, 3);

    // Width
    label = new QLabel("W:", this);
    label->setToolTip("Image width.");
    layout->addWidget(label, 1, 0);

    spinBoxWidth = new QSpinBox(this);
    spinBoxWidth->setKeyboardTracking(false);
    spinBoxWidth->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(spinBoxWidth, 1, 1);

    // Height
    label = new QLabel("H:", this);
    label->setToolTip("Image height.");
    layout->addWidget(label, 1, 2);
    
    spinBoxHeight = new QSpinBox(this);
    spinBoxHeight->setKeyboardTracking(false);
    spinBoxHeight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(spinBoxHeight, 1, 3);
}

SizeWidget::~SizeWidget ()
{
}


unicap_rect_t SizeWidget::getSize () const
{
    unicap_rect_t size;
    return size;
}
    

void SizeWidget::setSize (const unicap_rect_t &size)
{
    spinBoxOffsetX->setValue(size.x);
    spinBoxOffsetY->setValue(size.y);
    spinBoxWidth->setValue(size.width);
    spinBoxHeight->setValue(size.height);
}

void SizeWidget::setValidSizeRange (const unicap_rect_t &min_size, const unicap_rect_t &max_size, int step_h, int step_v)
{
    spinBoxOffsetX->setRange(min_size.x, max_size.x);
    spinBoxOffsetX->setSingleStep(step_h);

    spinBoxOffsetY->setRange(min_size.y, max_size.y);
    spinBoxOffsetY->setSingleStep(step_v);

    spinBoxWidth->setRange(min_size.width, max_size.width);
    spinBoxWidth->setSingleStep(step_h);

    spinBoxHeight->setRange(min_size.height, max_size.height);
    spinBoxHeight->setSingleStep(step_v);
}
