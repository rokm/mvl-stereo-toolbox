/*
 * Stereo Widgets: reprojection display widget
 * Copyright (C) 2013-2017 Rok Mandeljc
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

#ifndef MVL_STEREO_TOOLBOX__WIDGETS__REPROJECTION_DISPLAY_WIDGET_H
#define MVL_STEREO_TOOLBOX__WIDGETS__REPROJECTION_DISPLAY_WIDGET_H

#include <stereo-widgets/export.h>
#include <stereo-widgets/image_display_widget.h>


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


class ReprojectionDisplayWidgetPrivate;

class MVL_STEREO_WIDGETS_EXPORT ReprojectionDisplayWidget : public ImageDisplayWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ReprojectionDisplayWidget)
    Q_DECLARE_PRIVATE(ReprojectionDisplayWidget)

public:
    ReprojectionDisplayWidget (const QString &text = QString(), QWidget *parent = Q_NULLPTR);
    virtual ~ReprojectionDisplayWidget ();

    void setPoints (const cv::Mat &points);

protected:
    virtual void mouseMoveEvent (QMouseEvent *event) override;

    QVector3D getCoordinatesAtPixel (const QPoint &pos);

signals:
    void coordinatesUnderMouseChanged (QVector3D value);
};


} // Widgets
} // StereoToolbox
} // MVL


#endif
