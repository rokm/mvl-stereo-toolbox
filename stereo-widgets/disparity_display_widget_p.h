/*
 * Stereo Widgets: disparity display widget
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

#ifndef MVL_STEREO_TOOLBOX__WIDGETS__DISPARITY_DISPLAY_WIDGET_P_H
#define MVL_STEREO_TOOLBOX__WIDGETS__DISPARITY_DISPLAY_WIDGET_P_H

#include "image_display_widget_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


class DisparityDisplayWidgetPrivate : public ImageDisplayWidgetPrivate
{
    Q_DISABLE_COPY(DisparityDisplayWidgetPrivate)
    Q_DECLARE_PUBLIC(DisparityDisplayWidget)

protected:
    DisparityDisplayWidgetPrivate (DisparityDisplayWidget *parent);

protected:
    cv::Mat disparity;
};


} // Widgets
} // StereoToolbox
} // MVL


#endif
