/*
 * OpenCV Camera: config widget
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

#include "camera_widget.h"
#include "camera.h"

using namespace SourceOpenCvCam;


CameraWidget::CameraWidget (Camera *c, QWidget *parent)
    : QWidget(parent), camera(c)
{
    QFormLayout *layout = new QFormLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label;
    QComboBox *comboBox;
    QPushButton *button;
    QFrame *line;

    QString tooltip;

    connect(camera, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));
    connect(camera, SIGNAL(captureStarted()), this, SLOT(updateCameraState()));
    connect(camera, SIGNAL(captureFinished()), this, SLOT(updateCameraState()));

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

    // Update
    updateParameters();
}

CameraWidget::~CameraWidget ()
{
}


void CameraWidget::updateParameters ()
{
}


void CameraWidget::captureButtonToggled (bool start)
{
    if (start) {
        camera->startCapture();
    } else {
        camera->stopCapture();
    }
}


void CameraWidget::updateCameraState ()
{
    bool oldState = pushButtonCapture->blockSignals(true);
    pushButtonCapture->setChecked(camera->getCaptureState());
    pushButtonCapture->blockSignals(oldState);
}
