/*
 * Stereo Widgets: calibration pattern display widget
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

#ifndef MVL_STEREO_TOOLBOX__WIDGETS__CALIBRATION_PATTERN_DISPLAY_WIDGET_H
#define MVL_STEREO_TOOLBOX__WIDGETS__CALIBRATION_PATTERN_DISPLAY_WIDGET_H

#include <stereo-widgets/export.h>
#include <stereo-widgets/image_display_widget.h>


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


class CalibrationPatternDisplayWidgetPrivate;

class MVL_STEREO_WIDGETS_EXPORT CalibrationPatternDisplayWidget : public ImageDisplayWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(CalibrationPatternDisplayWidget)
    Q_DECLARE_PRIVATE(CalibrationPatternDisplayWidget)

public:
    CalibrationPatternDisplayWidget (const QString &text = QString(), QWidget *parent = Q_NULLPTR);
    virtual ~CalibrationPatternDisplayWidget ();

    void setPattern (bool found, const std::vector<cv::Point2f> &points, const cv::Size &size);

protected:
    virtual void paintEvent (QPaintEvent *event) override;

};


} // Widgets
} // StereoToolbox
} // MVL


#endif
