/*
 * Stereo Widgets: image pair display widget
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

#ifndef MVL_STEREO_TOOLBOX__WIDGETS__IMAGE_PAIR_DISPLAY_WIDGET_P_H
#define MVL_STEREO_TOOLBOX__WIDGETS__IMAGE_PAIR_DISPLAY_WIDGET_P_H

#include "image_display_widget_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


class ImagePairDisplayWidgetPrivate : public ImageDisplayWidgetPrivate
{
    Q_DISABLE_COPY(ImagePairDisplayWidgetPrivate)
    Q_DECLARE_PUBLIC(ImagePairDisplayWidget)

protected:
    ImagePairDisplayWidgetPrivate (ImagePairDisplayWidget *parent);

protected:
    bool displayPair;

    cv::Rect roiLeft;
    cv::Rect roiRight;

    cv::Mat imageLeft;
    cv::Mat imageRight;

    QPixmap pixmapLeft;
    QPixmap pixmapRight;
};


} // Widgets
} // StereoToolbox
} // MVL


#endif
