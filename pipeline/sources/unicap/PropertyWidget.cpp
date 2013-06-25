/*
 * Unicap Camera: generic property config widget
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

#include "PropertyWidget.h"

#include "CameraUnicap.h"


PropertyWidget::PropertyWidget (CameraUnicap *c, const unicap_property_t &p, QWidget *parent)
    : QWidget(parent), camera(c), property(p)
{
    connect(camera, SIGNAL(propertyChanged()), this, SLOT(updateProperty()));
    
    setLayout(new QHBoxLayout(this));
    layout()->setContentsMargins(0, 0, 0, 0);

    switch (property.type) {
        case UNICAP_PROPERTY_TYPE_RANGE: {
            if (property.range.min == 0 && property.range.max == 1 && property.stepping == 1) {
                // On/off; create a push button
                pushButtonValue = new QPushButton(this);
                pushButtonValue->setCheckable(true);

                connect(pushButtonValue, SIGNAL(toggled(bool)), this, SLOT(pushButtonValueToggled(bool)));
                layout()->addWidget(pushButtonValue);

                type = TypeOnOff;
            } else {
                // Spin box
                spinBoxValue = new QDoubleSpinBox(this);
                spinBoxValue->setToolTip(QString("Min: %1 Max: %2").arg(property.range.min).arg(property.range.max));
                spinBoxValue->setKeyboardTracking(false);
            
                spinBoxValue->setRange(property.range.min, property.range.max);
                spinBoxValue->setSingleStep(property.stepping);
                spinBoxValue->setValue(property.value);

                QString unit(property.unit);
                if (!unit.isEmpty()) {
                    spinBoxValue->setSuffix(" " + unit);
                }

                connect(spinBoxValue, SIGNAL(valueChanged(double)), this, SLOT(spinBoxValueChanged(double)));
                layout()->addWidget(spinBoxValue);

                type = TypeValue;
            }
            break;
        }
        case UNICAP_PROPERTY_TYPE_VALUE_LIST: {
            comboBoxValue = new QComboBox(this);

            connect(comboBoxValue, SIGNAL(activated(int)), this, SLOT(comboBoxValueActivated(int)));
            layout()->addWidget(comboBoxValue);

            for (int i = 0; i < property.value_list.value_count; i++) {
                comboBoxValue->addItem(QString::number(property.value_list.values[i]), property.value_list.values[i]);
            }

            type = TypeValueList;
            break;
        }
        case UNICAP_PROPERTY_TYPE_MENU: {
            comboBoxValue = new QComboBox(this);

            connect(comboBoxValue, SIGNAL(activated(int)), this, SLOT(comboBoxValueActivated(int)));
            layout()->addWidget(comboBoxValue);

            for (int i = 0; i < property.menu.menu_item_count; i++) {
                QString entry(property.menu.menu_items[i]);
                comboBoxValue->addItem(entry, entry);
            }

            type = TypeStringList;
            break;
        }
        default: {
            qWarning() << "Unhandled property type" << property.type;
            break;
        }
        
    }

    // Mode
    int numModes = 0;
    comboBoxMode = new QComboBox(this);
    connect(comboBoxMode, SIGNAL(activated(int)), this, SLOT(comboBoxModeActivated(int)));
    layout()->addWidget(comboBoxMode);

    if (property.flags_mask & UNICAP_FLAGS_MANUAL) {
        comboBoxMode->addItem("Manual", CameraUnicap::PropertyModeManual);
        numModes++;
    }
    
    if (property.flags_mask & UNICAP_FLAGS_AUTO) {
        comboBoxMode->addItem("Auto", CameraUnicap::PropertyModeAuto);
        numModes++;
    }

    if (property.flags_mask & UNICAP_FLAGS_ONE_PUSH) {
        comboBoxMode->addItem("One-push", CameraUnicap::PropertyModeOnePush);
        numModes++;
    }

    // Make visible only if there's at least two options
    if (numModes < 2) {
        comboBoxMode->hide();
    }  
    
    // Update parameter
    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateProperty()));
    updateTimer->start(1000);
    
    updateProperty();
}

PropertyWidget::~PropertyWidget ()
{
}


void PropertyWidget::updateProperty ()
{
    // Refresh property
    camera->updateProperty(property);

    bool oldState;

    // Value
    QWidget *valueWidget = NULL;
    switch (type) {
        case TypeValue: {
            oldState = spinBoxValue->blockSignals(true);
            if (property.value != spinBoxValue->value()) {
                spinBoxValue->setValue(property.value);
            }
            spinBoxValue->blockSignals(oldState);

            valueWidget = spinBoxValue;
            break;
        }
        case TypeValueList: {
            int idx = comboBoxValue->findData(property.value);

            oldState = comboBoxValue->blockSignals(true);
            comboBoxValue->setCurrentIndex(idx);
            comboBoxValue->blockSignals(oldState);

            valueWidget = comboBoxValue;
            break;
        }
        case TypeStringList: {
            int idx = comboBoxValue->findData(property.menu_item);

            oldState = comboBoxValue->blockSignals(true);
            comboBoxValue->setCurrentIndex(idx);
            comboBoxValue->blockSignals(oldState);

            valueWidget = comboBoxValue;
            break;
        }
        case TypeOnOff: {
            oldState = pushButtonValue->blockSignals(true);
            pushButtonValue->setChecked(property.value);
            pushButtonValue->setText(property.value ? "On" : "Off");
            pushButtonValue->blockSignals(oldState);

            valueWidget = pushButtonValue;
            break;
        }
        default: {
            qWarning() << "Unhandled type" << type;
            return;
        }
    }

    // Mode
    oldState = comboBoxMode->blockSignals(true);
    if (property.flags & UNICAP_FLAGS_MANUAL) {
        comboBoxMode->setCurrentIndex(comboBoxMode->findData(CameraUnicap::PropertyModeManual));
    } else if (property.flags & UNICAP_FLAGS_AUTO) {
        comboBoxMode->setCurrentIndex(comboBoxMode->findData(CameraUnicap::PropertyModeAuto));
    } else if (property.flags & UNICAP_FLAGS_ONE_PUSH) {
        comboBoxMode->setCurrentIndex(comboBoxMode->findData(CameraUnicap::PropertyModeOnePush));
    }
    comboBoxMode->blockSignals(oldState);
    
    if (property.flags & UNICAP_FLAGS_AUTO || property.flags & UNICAP_FLAGS_ONE_PUSH) {
        valueWidget->setEnabled(false);
    } else {
        valueWidget->setEnabled(true);
    }
}


void PropertyWidget::spinBoxValueChanged (double newValue)
{
    camera->setPropertyValue(property.identifier, newValue);
}

void PropertyWidget::comboBoxValueActivated (int index)
{
    if (type == TypeValueList) {
        camera->setPropertyValue(property.identifier, property.value_list.values[index]);
    } else if (type == TypeStringList) {
        camera->setPropertyValue(property.identifier, property.menu.menu_items[index]);
    }
}

void PropertyWidget::pushButtonValueToggled (bool newValue)
{
    camera->setPropertyValue(property.identifier, newValue);
}

void PropertyWidget::comboBoxModeActivated (int index)
{
    int mode = comboBoxMode->itemData(index).toInt();
    camera->setPropertyMode(property.identifier, (CameraUnicap::PropertyMode)mode);
}
