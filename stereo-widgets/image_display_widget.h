/*
 * Stereo Widgets: image display widget
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

#ifndef MVL_STEREO_TOOLBOX__WIDGETS__IMAGE_DISPLAY_WIDGET_H
#define MVL_STEREO_TOOLBOX__WIDGETS__IMAGE_DISPLAY_WIDGET_H

#include <stereo-widgets/export.h>

#include <QtWidgets>

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


class ImageDisplayWidgetPrivate;

class MVL_STEREO_WIDGETS_EXPORT ImageDisplayWidget : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(ImageDisplayWidget)
    Q_DECLARE_PRIVATE(ImageDisplayWidget)

protected:
    QScopedPointer<ImageDisplayWidgetPrivate> const d_ptr;
    ImageDisplayWidget (ImageDisplayWidgetPrivate *d, const QString &text = QString(), QWidget *parent = nullptr);

public:
    ImageDisplayWidget (const QString &text = QString(), QWidget *parent = nullptr);
    virtual ~ImageDisplayWidget ();

    virtual void setImage (const cv::Mat &image);
    void setText (const QString &text);

    static QImage convertCvMatToQImage (const cv::Mat &src);

protected:
    virtual void paintEvent (QPaintEvent *event) override;
};


} // Widgets
} // StereoToolbox
} // MVL


#endif
