/*
 * DC1394 Camera: generic feature config widget
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

#include "FeatureWidget.h"

#include "CameraDC1394.h"


static const QString featureModeToString (dc1394feature_mode_t);


FeatureWidget::FeatureWidget (CameraDC1394 *c, const dc1394feature_info_t &f, QWidget *parent)
    : QWidget(parent), camera(c), feature(f)
{
    connect(camera, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));
    
    setLayout(new QHBoxLayout(this));
    layout()->setContentsMargins(0, 0, 0, 0);
    
    // Value
    spinBoxValue = new QSpinBox(this);
    spinBoxValue->setToolTip(QString("Min: %1 Max: %2").arg(feature.min).arg(feature.max));
    spinBoxValue->setKeyboardTracking(false);
    spinBoxValue->setRange(feature.min, feature.max);
    connect(spinBoxValue, SIGNAL(valueChanged(int)), this, SLOT(setValue(int)));
    layout()->addWidget(spinBoxValue);

    // Mode
    comboBoxMode = new QComboBox(this);
    connect(comboBoxMode, SIGNAL(activated(int)), this, SLOT(modeChanged(int)));
    layout()->addWidget(comboBoxMode);

    QList<dc1394feature_mode_t> availableModes = camera->getFeatureModes(feature.id);
    foreach (dc1394feature_mode_t mode, availableModes) {
        comboBoxMode->addItem(featureModeToString(mode), mode);
    }

    // Make visible only if there's at least two options
    if (availableModes.size() < 2) {
        comboBoxMode->hide();
    }
    
    // Absolute value
    if (feature.absolute_capable) {
        spinBoxAbsoluteValue = new QDoubleSpinBox(this);
        spinBoxAbsoluteValue->setKeyboardTracking(false);
        spinBoxAbsoluteValue->setDecimals(6);
        spinBoxAbsoluteValue->setEnabled(false);
        spinBoxAbsoluteValue->setRange(feature.abs_min, feature.abs_max);
        connect(spinBoxAbsoluteValue, SIGNAL(valueChanged(double)), this, SLOT(setAbsoluteValue(double)));
        layout()->addWidget(spinBoxAbsoluteValue);
    }

    // Update parameter
    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateParameters()));
    
    updateParameters();
}

FeatureWidget::~FeatureWidget ()
{
}


void FeatureWidget::updateParameters ()
{
    bool oldState;
    
    // Value
    oldState = spinBoxValue->blockSignals(true);
    spinBoxValue->setValue(camera->getFeatureValue(feature.id));
    spinBoxValue->blockSignals(oldState);

    // Mode
    oldState = comboBoxMode->blockSignals(true);
    comboBoxMode->setCurrentIndex(comboBoxMode->findData(camera->getFeatureMode(feature.id)));
    comboBoxMode->blockSignals(oldState);

    // Absolute value
    if (feature.absolute_capable) {
        oldState = spinBoxAbsoluteValue->blockSignals(true);
        spinBoxAbsoluteValue->setValue(camera->getFeatureAbsoluteValue(feature.id));
        spinBoxAbsoluteValue->blockSignals(oldState);
    }

    // If mode is auto, disable the controls and enable the refresh timer,
    // otherwise, disable timer and enable control
    if (comboBoxMode->itemData(comboBoxMode->currentIndex()) == DC1394_FEATURE_MODE_AUTO) {
        spinBoxValue->setEnabled(false);
        updateTimer->start(1000);
    } else {
        spinBoxValue->setEnabled(true);
        updateTimer->stop();
    }
}


void FeatureWidget::setValue (int newValue)
{
    camera->setFeatureValue(feature.id, newValue);
}

void FeatureWidget::setAbsoluteValue (double newValue)
{
    camera->setFeatureAbsoluteValue(feature.id, newValue);
}

void FeatureWidget::modeChanged (int index)
{
    camera->setFeatureMode(feature.id, (dc1394feature_mode_t)comboBoxMode->itemData(index).toInt());
}


// *********************************************************************
// *              libdc1394 constant to string conversion              *
// *********************************************************************
static const QString featureModeToString (dc1394feature_mode_t mode)
{
    switch (mode) {
        case DC1394_FEATURE_MODE_MANUAL: return "Manual";
        case DC1394_FEATURE_MODE_AUTO: return "Auto";
        case DC1394_FEATURE_MODE_ONE_PUSH_AUTO: return "One push auto";  
    }

    return "INVALID";
}
