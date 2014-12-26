/*
 * OpenCV Camera: generic property config widget
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

#include "property_widget.h"
#include "camera.h"

using namespace SourceOpenCvCam;


PropertyWidget::PropertyWidget (Camera *c, int p, bool integer_value, QWidget *parent)
    : QDoubleSpinBox(parent), camera(c), property(p)
{
    // Integer vs. double
    if (integer_value) {
        setRange(-9999, 9999);
        setDecimals(0);
    } else {
        setRange(0, 1.0);
        setDecimals(6);
        setSingleStep(0.01);
    }

    setKeyboardTracking(false);

    connect(camera, &Camera::propertyChanged, this, &PropertyWidget::updateProperty);

    connect(this, static_cast<void (PropertyWidget::*)(double)>(&PropertyWidget::valueChanged), this, [this] (double newValue) {
        camera->setProperty(property, newValue);
    });

    // Get initial property value; as per OpenCV docs, assume that if 0
    // is returned, property is not supported. Additionally, tests
    // indicate that sometimes -1 is returned for missing controls
    // (at least on linux with V4L)
    double initialValue = camera->getProperty(property);
    if (initialValue == -1.0  || initialValue == 0.0) {
        setEnabled(false);
    }

    // Update parameter
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &PropertyWidget::updateProperty);
    updateTimer->start(1000);

    updateProperty();
}

PropertyWidget::~PropertyWidget ()
{
}


void PropertyWidget::updateProperty ()
{
    if (isEnabled() && !hasFocus()) {
        blockSignals(true);
        setValue(camera->getProperty(property));
        blockSignals(false);
    }
}
