/*
 * Stereo Widgets: image display widget
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

#ifndef MVL_STEREO_TOOLBOX__WIDGETS__IMAGE_DISPLAY_WIDGET_P_H
#define MVL_STEREO_TOOLBOX__WIDGETS__IMAGE_DISPLAY_WIDGET_P_H


#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


class ImageDisplayWidgetPrivate
{
    Q_DISABLE_COPY(ImageDisplayWidgetPrivate)
    Q_DECLARE_PUBLIC(ImageDisplayWidget)

protected:
    ImageDisplayWidgetPrivate (ImageDisplayWidget *parent);
    ImageDisplayWidget * const q_ptr;

protected:
    QString text;

    cv::Mat image;
};


} // Widgets
} // StereoToolbox
} // MVL


#endif
