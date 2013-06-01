/*
 * DC1394 Camera: config widget
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

#include "CameraDC1394ConfigWidget.h"
#include "CameraDC1394.h"

#include "FeatureWidget.h"


static const QString videoModeToString (dc1394video_mode_t);
static const QString framerateToString (dc1394framerate_t);


CameraDC1394ConfigWidget::CameraDC1394ConfigWidget (CameraDC1394 *c, QWidget *parent)
    : QWidget(parent), camera(c)
{
    QFormLayout *layout = new QFormLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label;
    QComboBox *comboBox;
    QFrame *line;

    QString tooltip;

    connect(camera, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

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

CameraDC1394ConfigWidget::~CameraDC1394ConfigWidget ()
{
}

void CameraDC1394ConfigWidget::addFeatureWidgets ()
{
    const dc1394featureset_t &featureSet = camera->getFeatureSet();

    QLabel *label;
    FeatureWidget *widget;

    for (int i = 0; i < sizeof(featureSet.feature)/sizeof(featureSet.feature[0]); i++) {
        const dc1394feature_info_t &feature = featureSet.feature[i];
        
        if (feature.available) {
            label = new QLabel(dc1394_feature_get_string(feature.id), this);
            widget = new FeatureWidget(camera, feature, this);

            qobject_cast<QFormLayout *>(layout())->addRow(label, widget);
        }

    }
}



void CameraDC1394ConfigWidget::modeChanged (int index)
{
    camera->setMode((dc1394video_mode_t)comboBoxMode->itemData(index).toInt());

    // Mode change requires re-enumeration of framerates
    comboBoxFramerate->clear();
    foreach (dc1394framerate_t framerate, camera->getSupportedFramerates()) {
        comboBoxFramerate->addItem(framerateToString(framerate), framerate);
    }
}


void CameraDC1394ConfigWidget::framerateChanged (int index)
{
    camera->setFramerate((dc1394framerate_t)comboBoxFramerate->itemData(index).toInt());
}



void CameraDC1394ConfigWidget::updateParameters ()
{
    comboBoxMode->setCurrentIndex(comboBoxMode->findData(camera->getMode()));
    comboBoxFramerate->setCurrentIndex(comboBoxFramerate->findData(camera->getFramerate()));
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
}
