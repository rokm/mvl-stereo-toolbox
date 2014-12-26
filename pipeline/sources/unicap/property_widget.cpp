/*
 * Unicap Camera: generic property config widget
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

using namespace SourceUnicap;


PropertyWidget::PropertyWidget (Camera *c, const unicap_property_t &p, QWidget *parent)
    : QWidget(parent), camera(c), property(p)
{
    connect(camera, &Camera::propertyChanged, this, &PropertyWidget::updateProperty);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    switch (property.type) {
        case UNICAP_PROPERTY_TYPE_RANGE: {
            if (property.range.min == 0 && property.range.max == 1 && property.stepping == 1) {
                // On/off; create a push button
                pushButtonValue = new QPushButton(this);
                pushButtonValue->setCheckable(true);

                connect(pushButtonValue, &QPushButton::toggled, this, [this] (bool newValue) {
                    camera->setPropertyValue(property.identifier, newValue);
                });
                layout->addWidget(pushButtonValue);

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

                connect(spinBoxValue, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this] (double newValue) {
                    camera->setPropertyValue(property.identifier, newValue);
                });
                layout->addWidget(spinBoxValue);

                type = TypeValue;
            }
            break;
        }
        case UNICAP_PROPERTY_TYPE_VALUE_LIST: {
            comboBoxValue = new QComboBox(this);

            connect(comboBoxValue, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [this] (int index) {
                camera->setPropertyValue(property.identifier, property.value_list.values[index]);
            });
            layout->addWidget(comboBoxValue);

            for (int i = 0; i < property.value_list.value_count; i++) {
                comboBoxValue->addItem(QString::number(property.value_list.values[i]), property.value_list.values[i]);
            }

            type = TypeValueList;
            break;
        }
        case UNICAP_PROPERTY_TYPE_MENU: {
            comboBoxValue = new QComboBox(this);

            connect(comboBoxValue, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [this] (int index) {
                camera->setPropertyValue(property.identifier, property.menu.menu_items[index]);
            });
            layout->addWidget(comboBoxValue);

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
    connect(comboBoxMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [this] (int index) {
        int mode = comboBoxMode->itemData(index).toInt();
        camera->setPropertyMode(property.identifier, (Camera::PropertyMode)mode);
    });
    layout->addWidget(comboBoxMode);

    if (property.flags_mask & UNICAP_FLAGS_MANUAL) {
        comboBoxMode->addItem("Manual", Camera::PropertyModeManual);
        numModes++;
    }
    
    if (property.flags_mask & UNICAP_FLAGS_AUTO) {
        comboBoxMode->addItem("Auto", Camera::PropertyModeAuto);
        numModes++;
    }

    if (property.flags_mask & UNICAP_FLAGS_ONE_PUSH) {
        comboBoxMode->addItem("One-push", Camera::PropertyModeOnePush);
        numModes++;
    }

    // Make visible only if there's at least two options
    if (numModes < 2) {
        comboBoxMode->hide();
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
    // Refresh property
    camera->updateProperty(property);

    // Value
    QWidget *valueWidget = NULL;
    switch (type) {
        case TypeValue: {
            spinBoxValue->blockSignals(true);
            if (spinBoxValue->hasFocus()) {
                spinBoxValue->setValue(property.value);
            }
            spinBoxValue->blockSignals(false);

            valueWidget = spinBoxValue;
            break;
        }
        case TypeValueList: {
            int idx = comboBoxValue->findData(property.value);

            comboBoxValue->blockSignals(true);
            comboBoxValue->setCurrentIndex(idx);
            comboBoxValue->blockSignals(false);

            valueWidget = comboBoxValue;
            break;
        }
        case TypeStringList: {
            int idx = comboBoxValue->findData(property.menu_item);

            comboBoxValue->blockSignals(true);
            comboBoxValue->setCurrentIndex(idx);
            comboBoxValue->blockSignals(false);

            valueWidget = comboBoxValue;
            break;
        }
        case TypeOnOff: {
            pushButtonValue->blockSignals(true);
            pushButtonValue->setChecked(property.value);
            pushButtonValue->setText(property.value ? "On" : "Off");
            pushButtonValue->blockSignals(false);

            valueWidget = pushButtonValue;
            break;
        }
        default: {
            qWarning() << "Unhandled type" << type;
            return;
        }
    }

    // Mode
    comboBoxMode->blockSignals(true);
    if (property.flags & UNICAP_FLAGS_MANUAL) {
        comboBoxMode->setCurrentIndex(comboBoxMode->findData(Camera::PropertyModeManual));
    } else if (property.flags & UNICAP_FLAGS_AUTO) {
        comboBoxMode->setCurrentIndex(comboBoxMode->findData(Camera::PropertyModeAuto));
    } else if (property.flags & UNICAP_FLAGS_ONE_PUSH) {
        comboBoxMode->setCurrentIndex(comboBoxMode->findData(Camera::PropertyModeOnePush));
    }
    comboBoxMode->blockSignals(false);
    
    if (property.flags & UNICAP_FLAGS_AUTO || property.flags & UNICAP_FLAGS_ONE_PUSH) {
        valueWidget->setEnabled(false);
    } else {
        valueWidget->setEnabled(true);
    }
}
