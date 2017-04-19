/*
 * Unicap Source: source widget
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__UNICAP__SOURCE_WIDGET_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__UNICAP__SOURCE_WIDGET_H

#include <QtWidgets>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceUnicap {


class Source;
class Camera;


class CameraFrame;


class SourceWidget : public QWidget
{
    Q_OBJECT

public:
    SourceWidget (Source *source, QWidget *parent = 0);
    virtual ~SourceWidget ();

protected:
    CameraFrame *frameLeftCamera;
    CameraFrame *frameRightCamera;
};



class CameraFrame : public QFrame
{
    Q_OBJECT

public:
    CameraFrame (Source *source, QWidget *parent);
    virtual ~CameraFrame ();

    void setCamera (Camera *camera);

    void setLabel (const QString &text);
    void setComboBoxToolip (const QString &text);

signals:
    void deviceSelected (int index);

protected:
    QLabel *labelCamera;
    QComboBox *comboBoxCamera;

    QFrame *frameCamera;
    QWidget *widgetCameraConfig;
};


} // SourceUnicap
} // Pipeline
} // StereoToolbox
} // MVL


#endif
