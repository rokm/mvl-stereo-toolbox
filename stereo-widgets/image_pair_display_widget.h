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

#ifndef MVL_STEREO_TOOLBOX__WIDGETS__IMAGE_PAIR_DISPLAY_WIDGET_H
#define MVL_STEREO_TOOLBOX__WIDGETS__IMAGE_PAIR_DISPLAY_WIDGET_H

#include <stereo-widgets/export.h>
#include <stereo-widgets/image_display_widget.h>


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


class ImagePairDisplayWidgetPrivate;

class MVL_STEREO_WIDGETS_EXPORT ImagePairDisplayWidget : public ImageDisplayWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ImagePairDisplayWidget)
    Q_DECLARE_PRIVATE(ImagePairDisplayWidget)

public:
    ImagePairDisplayWidget (const QString &text = QString(), QWidget *parent = nullptr);
    virtual ~ImagePairDisplayWidget ();

    virtual void setImage (const cv::Mat &image) override;

    void setImagePairROI (const cv::Rect &roiLeft, const cv::Rect &roiRight);
    void setImagePair (const cv::Mat &left, const cv::Mat &right);

protected:
    virtual void paintEvent (QPaintEvent *event) override;
};


} // Widgets
} // StereoToolbox
} // MVL


#endif
