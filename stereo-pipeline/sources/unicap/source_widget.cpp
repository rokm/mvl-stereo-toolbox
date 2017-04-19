/*
 * Unicap Source: source widget
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
#include "camera.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceUnicap {


// *********************************************************************
// *                           Source widget                           *
// *********************************************************************
SourceWidget::SourceWidget (Source *source, QWidget *parent)
    : QWidget(parent)
{
    // Build layout
    QVBoxLayout *baseLayout = new QVBoxLayout(this);

    QLabel *label;
    QFrame *line;
    QHBoxLayout *hbox;
    QPushButton *button;
    QComboBox *comboBox;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>Unicap source</u></b>", this);
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

    // Single/dual camera mode switch
    comboBox = new QComboBox(this);

    comboBox->addItem("Single-camera mode");
    comboBox->addItem("Dual-camera mode");

    connect(comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this, comboBox, source] (int index) {
        source->setSingleCameraMode(index == 0);
    });

    connect(source, &Source::singleCameraModeChanged, this, [this, comboBox] (bool enabled) {
        // Update combo box
        comboBox->blockSignals(true);
        comboBox->setCurrentIndex(!enabled);
        comboBox->blockSignals(false);

        // Update camera frames
        if (enabled) {
            frameLeftCamera->setLabel("<b>Combined camera</b>");
            frameRightCamera->hide();
        } else {
            frameLeftCamera->setLabel("<b>Left camera</b>");
            frameRightCamera->show();
        }
    });
    comboBox->setCurrentIndex(!source->getSingleCameraMode());

    layout->addRow(comboBox);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

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

    // *** Cameras ***
    hbox = new QHBoxLayout();
    layout->addRow(hbox);

    // Left
    frameLeftCamera = new CameraFrame(source, this);
    frameLeftCamera->setLabel("<b>Left camera</b>");

    connect(frameLeftCamera, &CameraFrame::deviceSelected, this, [this, source] (int device) {
        source->setLeftCamera(device);
    });

    connect(source, &Source::leftCameraChanged, this, [this, source] () {
        frameLeftCamera->setCamera(source->getLeftCamera());
    });

    hbox->addWidget(frameLeftCamera);

    // Right
    frameRightCamera = new CameraFrame(source, this);
    frameRightCamera->setLabel("<b>Right camera</b>");

    connect(frameRightCamera, &CameraFrame::deviceSelected, this, [this, source] (int device) {
        source->setRightCamera(device);
    });

    connect(source, &Source::rightCameraChanged, this, [this, source] () {
        frameRightCamera->setCamera(source->getRightCamera());
    });

    hbox->addWidget(frameRightCamera);
}

SourceWidget::~SourceWidget ()
{
}


// *********************************************************************
// *                           Camera frame                            *
// *********************************************************************
CameraFrame::CameraFrame (Source *source, QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle(QFrame::Box | QFrame::Sunken);

    widgetCameraConfig = nullptr;

    QFormLayout *layout = new QFormLayout(this);

    // Label
    labelCamera = new QLabel(this);
    labelCamera->setAlignment(Qt::AlignCenter);
    layout->addRow(labelCamera);

    // Combo box
    comboBoxCamera = new QComboBox(this);
    comboBoxCamera->setModel(source);
    connect(comboBoxCamera, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [this] (int index) {
        QVariant item = comboBoxCamera->itemData(index);
        int device = item.isValid() ? item.toInt() : -1;
        emit deviceSelected(device);
    });
    layout->addRow(comboBoxCamera);

    // Camera config frame
    frameCamera = new QFrame(this);
    frameCamera->setContentsMargins(0, 0, 0, 0);
    layout->addRow(frameCamera);

    QVBoxLayout *frameLayout = new QVBoxLayout(frameCamera);
    frameLayout->setContentsMargins(0, 0, 0, 0);
}

CameraFrame::~CameraFrame ()
{
}


void CameraFrame::setCamera (Camera *camera)
{
    // Remove config widget for old camera
    if (widgetCameraConfig) {
        frameCamera->layout()->removeWidget(widgetCameraConfig);
        widgetCameraConfig->deleteLater();
        widgetCameraConfig = nullptr;
    }

    // Get new device's config widget
    if (camera) {
        widgetCameraConfig = camera->createConfigWidget(this);
        frameCamera->layout()->addWidget(widgetCameraConfig);
    }
}

void CameraFrame::setLabel (const QString &text)
{
    labelCamera->setText(text);
}


} // SourceUnicap
} // Pipeline
} // StereoToolbox
} // MVL
