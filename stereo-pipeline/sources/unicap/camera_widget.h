/*
 * Unicap Source: camera widget
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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__UNICAP__CAMERA_WIDGET_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__UNICAP__CAMERA_WIDGET_H

#include <QtWidgets>
#include <unicap.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceUnicap {


class Camera;
class SizeWidget;

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    CameraWidget (Camera *c, QWidget *parent = nullptr);
    virtual ~CameraWidget ();

protected:
    void updateCameraState ();

    void updateFormat ();
    void updateSize ();

    void addPropertyWidgets ();

protected:
    Camera *camera;

    QPushButton *pushButtonCapture;

    QComboBox *comboBoxFormat;

    unicap_format_t format;

    QLabel *labelSize1;
    QComboBox *comboBoxSize;

    QLabel *labelSize2;
    SizeWidget *widgetSize;
};


class SizeWidget : public QWidget
{
    Q_OBJECT

public:
    SizeWidget (QWidget *parent = nullptr);
    virtual ~SizeWidget ();

    void setSize (const unicap_rect_t &size);
    unicap_rect_t getSize () const;

    void setValidSizeRange (const unicap_rect_t &min_size, const unicap_rect_t &max_size, int step_w, int step_h);

signals:
    void sizeChanged (unicap_rect_t size);

protected:
    QSpinBox *spinBoxOffsetX;
    QSpinBox *spinBoxOffsetY;
    QSpinBox *spinBoxWidth;
    QSpinBox *spinBoxHeight;
};


} // SourceUnicap
} // Pipeline
} // StereoToolbox
} // MVL


#endif
