/*
 * DC1394 Camera: config widget
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
#include "feature_widget.h"

using namespace SourceDC1394;


static const QString videoModeToString (dc1394video_mode_t);
static const QString framerateToString (dc1394framerate_t);


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

    // Vendor
    tooltip = "Camera vendor.";
    
    label = new QLabel("<b>Vendor: </b>" + camera->getVendor(), this);
    label->setToolTip(tooltip);

    layout->addRow(label);

    // Model
    tooltip = "Camera model.";
    
    label = new QLabel("<b>Model: </b>" + camera->getModel(), this);
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

    // Mode
    tooltip = "Camera resolution and color mode.";
    
    label = new QLabel("Mode", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(modeChanged(int)));
    comboBoxMode = comboBox;
    
    layout->addRow(label, comboBox);
    
    foreach (dc1394video_mode_t mode, camera->getSupportedModes()) {
        comboBoxMode->addItem(videoModeToString(mode), mode);
    }

    // Framerate
    tooltip = "Framerate.";
    
    label = new QLabel("Framerate", this);
    label->setToolTip(tooltip);

    comboBox = new QComboBox(this);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(framerateChanged(int)));
    comboBoxFramerate = comboBox;
    
    layout->addRow(label, comboBox);
    
    foreach (dc1394framerate_t framerate, camera->getSupportedFramerates()) {
        comboBoxFramerate->addItem(framerateToString(framerate), framerate);
    }

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Feature widgets
    addFeatureWidgets();

    // Update
    updateParameters();
}

CameraWidget::~CameraWidget ()
{
}

void CameraWidget::addFeatureWidgets ()
{
    const dc1394featureset_t &featureSet = camera->getFeatureSet();

    QLabel *label;
    FeatureWidget *widget;

    for (unsigned int i = 0; i < sizeof(featureSet.feature)/sizeof(featureSet.feature[0]); i++) {
        const dc1394feature_info_t &feature = featureSet.feature[i];
        
        if (feature.available) {
            label = new QLabel(dc1394_feature_get_string(feature.id), this);
            widget = new FeatureWidget(camera, feature, this);

            qobject_cast<QFormLayout *>(layout())->addRow(label, widget);
        }
    }
}



void CameraWidget::modeChanged (int index)
{
    camera->setMode((dc1394video_mode_t)comboBoxMode->itemData(index).toInt());

    // Mode change requires re-enumeration of framerates
    comboBoxFramerate->clear();
    foreach (dc1394framerate_t framerate, camera->getSupportedFramerates()) {
        comboBoxFramerate->addItem(framerateToString(framerate), framerate);
    }
}


void CameraWidget::framerateChanged (int index)
{
    camera->setFramerate((dc1394framerate_t)comboBoxFramerate->itemData(index).toInt());
}



void CameraWidget::updateParameters ()
{
    bool oldState;

    // Mode
    oldState = comboBoxMode->blockSignals(true);
    comboBoxMode->setCurrentIndex(comboBoxMode->findData(camera->getMode()));
    comboBoxMode->blockSignals(oldState);

    // Framerate
    oldState = comboBoxFramerate->blockSignals(true);
    comboBoxFramerate->setCurrentIndex(comboBoxFramerate->findData(camera->getFramerate()));
    comboBoxFramerate->blockSignals(oldState);
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
    pushButtonCapture->setChecked(camera->getCaptureState());
}


// *********************************************************************
// *              libdc1394 constant to string conversion              *
// *********************************************************************
#define CASE_ENTRY(X) case X: return #X;

static const QString videoModeToString (dc1394video_mode_t mode)
{
    switch (mode) {
        CASE_ENTRY(DC1394_VIDEO_MODE_160x120_YUV444)
        CASE_ENTRY(DC1394_VIDEO_MODE_320x240_YUV422)
        CASE_ENTRY(DC1394_VIDEO_MODE_640x480_YUV411)
        CASE_ENTRY(DC1394_VIDEO_MODE_640x480_YUV422)
        CASE_ENTRY(DC1394_VIDEO_MODE_640x480_RGB8)
        CASE_ENTRY(DC1394_VIDEO_MODE_640x480_MONO8)
        CASE_ENTRY(DC1394_VIDEO_MODE_640x480_MONO16)
        CASE_ENTRY(DC1394_VIDEO_MODE_800x600_YUV422)
        CASE_ENTRY(DC1394_VIDEO_MODE_800x600_RGB8)
        CASE_ENTRY(DC1394_VIDEO_MODE_800x600_MONO8)
        CASE_ENTRY(DC1394_VIDEO_MODE_1024x768_YUV422)
        CASE_ENTRY(DC1394_VIDEO_MODE_1024x768_RGB8)
        CASE_ENTRY(DC1394_VIDEO_MODE_1024x768_MONO8)
        CASE_ENTRY(DC1394_VIDEO_MODE_800x600_MONO16)
        CASE_ENTRY(DC1394_VIDEO_MODE_1024x768_MONO16)
        CASE_ENTRY(DC1394_VIDEO_MODE_1280x960_YUV422)
        CASE_ENTRY(DC1394_VIDEO_MODE_1280x960_RGB8)
        CASE_ENTRY(DC1394_VIDEO_MODE_1280x960_MONO8)
        CASE_ENTRY(DC1394_VIDEO_MODE_1600x1200_YUV422)
        CASE_ENTRY(DC1394_VIDEO_MODE_1600x1200_RGB8)
        CASE_ENTRY(DC1394_VIDEO_MODE_1600x1200_MONO8)
        CASE_ENTRY(DC1394_VIDEO_MODE_1280x960_MONO16)
        CASE_ENTRY(DC1394_VIDEO_MODE_1600x1200_MONO16)
        CASE_ENTRY(DC1394_VIDEO_MODE_EXIF)
        CASE_ENTRY(DC1394_VIDEO_MODE_FORMAT7_0)
        CASE_ENTRY(DC1394_VIDEO_MODE_FORMAT7_1)
        CASE_ENTRY(DC1394_VIDEO_MODE_FORMAT7_2)
        CASE_ENTRY(DC1394_VIDEO_MODE_FORMAT7_3)
        CASE_ENTRY(DC1394_VIDEO_MODE_FORMAT7_4)
        CASE_ENTRY(DC1394_VIDEO_MODE_FORMAT7_5)
        CASE_ENTRY(DC1394_VIDEO_MODE_FORMAT7_6)
        CASE_ENTRY(DC1394_VIDEO_MODE_FORMAT7_7)
    }

    return "INVALID";
}

static const QString framerateToString (dc1394framerate_t framerate)
{
    switch (framerate) {
        CASE_ENTRY(DC1394_FRAMERATE_1_875)
        CASE_ENTRY(DC1394_FRAMERATE_3_75)
        CASE_ENTRY(DC1394_FRAMERATE_7_5)
        CASE_ENTRY(DC1394_FRAMERATE_15)
        CASE_ENTRY(DC1394_FRAMERATE_30)
        CASE_ENTRY(DC1394_FRAMERATE_60)
        CASE_ENTRY(DC1394_FRAMERATE_120)
        CASE_ENTRY(DC1394_FRAMERATE_240)
    }

    return "INVALID";
}
