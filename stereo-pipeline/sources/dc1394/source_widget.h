/*
 * DC1394 Source: source widget
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__DC1394__SOURCE_WIDGET_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__DC1394__SOURCE_WIDGET_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceDC1394 {


class Source;
class Camera;

class SourceWidget : public QWidget
{
    Q_OBJECT

public:
    SourceWidget (Source *, QWidget * = 0);
    virtual ~SourceWidget ();

protected:
    void deviceSelected (int);

    QWidget *createDeviceFrame (bool);

    void updateCamera (QWidget *&, QFrame *&, Camera *);

protected:
    Source *source;

    // GUI
    QComboBox *comboBoxLeftDevice;
    QComboBox *comboBoxRightDevice;

    QPushButton *pushButtonCaptureLeftDevice;
    QPushButton *pushButtonCaptureRightDevice;

    QHBoxLayout *boxDevices;

    QFrame *frameLeftDevice;
    QFrame *frameRightDevice;

    QWidget *configLeftDevice;
    QWidget *configRightDevice;
};


} // SourceDC1394
} // Pipeline
} // StereoToolbox
} // MVL


#endif