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

#ifndef PROPERTY_WIDGET_H
#define PROPERTY_WIDGET_H

#include <QtWidgets>
#include <unicap.h>


namespace SourceUnicap {

class Camera;

class PropertyWidget : public QWidget
{
    Q_OBJECT

public:
    PropertyWidget (Camera *, const unicap_property_t &, QWidget * = 0);
    virtual ~PropertyWidget ();

protected slots:
    void spinBoxValueChanged (double);
    void comboBoxValueActivated (int);
    void pushButtonValueToggled (bool);

    void comboBoxModeActivated (int);

    void updateProperty ();

protected:
    Camera *camera;
    unicap_property_t property;

    QTimer *updateTimer;

    enum {
        TypeValue,
        TypeValueList,
        TypeStringList,
        TypeOnOff,
    } type;

    QDoubleSpinBox *spinBoxValue;
    QComboBox *comboBoxValue;
    QPushButton *pushButtonValue;

    QComboBox *comboBoxMode;
};

}

#endif
