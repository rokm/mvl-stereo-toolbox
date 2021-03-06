/*
 * OpenCV Camera Source: camera widget
 * Copyright (C) 2013-2017 Rok Mandeljc
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
#include "property_widget.h"


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceOpenCvCam {


CameraWidget::CameraWidget (Camera *camera, QWidget *parent)
    : QWidget(parent),
      camera(camera)
{
    QFormLayout *layout = new QFormLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    QPushButton *button;
    QFrame *line;

    QString tooltip;

    connect(camera, &Camera::captureStarted, this, &CameraWidget::updateCameraState, Qt::QueuedConnection);
    connect(camera, &Camera::captureFinished, this, &CameraWidget::updateCameraState, Qt::QueuedConnection);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Capture
    tooltip = "Start/stop capture.";

    button = new QPushButton("Capture", this);
    button->setToolTip(tooltip);
    button->setCheckable(true);
    connect(button, &QPushButton::toggled, camera, [this] (bool start) {
        if (start) {
            this->camera->startCapture();
        } else {
            this->camera->stopCapture();
        }
    }, Qt::QueuedConnection);
    pushButtonCapture = button;

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Property widgets
    addPropertyWidgets();
}

CameraWidget::~CameraWidget ()
{
}


void CameraWidget::addPropertyWidgets ()
{
    static const struct {
        int prop;
        const char *name;
        bool integer_value;
    } properties[] = {
        { cv::CAP_PROP_FRAME_WIDTH, "Frame width", true },
        { cv::CAP_PROP_FRAME_HEIGHT, "Frame height", true },
        //{ cv::CAP_PROP_FPS, "FPS", false },
        { cv::CAP_PROP_BRIGHTNESS, "Brightness", false },
        { cv::CAP_PROP_CONTRAST, "Contrast", false },
        { cv::CAP_PROP_SATURATION, "Saturation", false },
        { cv::CAP_PROP_HUE, "Hue", false },
        { cv::CAP_PROP_GAIN, "Gain", false },
        { cv::CAP_PROP_EXPOSURE, "Exposure", false },
    };

    QLabel *label;
    PropertyWidget *widget;

    for (unsigned int i = 0; i < sizeof(properties)/sizeof(properties[0]); i++) {
        label = new QLabel(properties[i].name, this);
        widget = new PropertyWidget(camera, properties[i].prop, properties[i].integer_value, this);

        qobject_cast<QFormLayout *>(layout())->addRow(label, widget);
    }
}


void CameraWidget::updateCameraState ()
{
    pushButtonCapture->blockSignals(true);
    pushButtonCapture->setChecked(camera->getCaptureState());
    pushButtonCapture->blockSignals(false);
}


} // SourceOpenCvCam
} // Pipeline
} // StereoToolbox
} // MVL
