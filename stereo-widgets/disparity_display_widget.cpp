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

#include "disparity_display_widget.h"
#include "disparity_display_widget_p.h"

#include <limits>


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


DisparityDisplayWidgetPrivate::DisparityDisplayWidgetPrivate (DisparityDisplayWidget *parent)
    : ImageDisplayWidgetPrivate(parent)
{
}


DisparityDisplayWidget::DisparityDisplayWidget (const QString &text, QWidget *parent)
    : ImageDisplayWidget(new DisparityDisplayWidgetPrivate(this), text, parent)
{
    setMouseTracking(true); // Enable mouse tracking
}

DisparityDisplayWidget::~DisparityDisplayWidget ()
{
}


void DisparityDisplayWidget::mouseMoveEvent (QMouseEvent *event)
{
    emit disparityUnderMouseChanged(getDisparityAtPixel(event->pos()));

    // Chain up to parent
    return ImageDisplayWidget::mouseMoveEvent(event);
}


void DisparityDisplayWidget::setDisparity (const cv::Mat &disparity)
{
    Q_D(DisparityDisplayWidget);

    d->disparity = disparity;
    emit disparityUnderMouseChanged(getDisparityAtPixel(mapFromGlobal(QCursor::pos())));
}


float DisparityDisplayWidget::getDisparityAtPixel (const QPoint &pos)
{
    Q_D(DisparityDisplayWidget);

    // Make sure there is image displayed
    if (d->disparity.empty()) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    // Validate dimensions
    if (d->image.width() != d->disparity.cols || d->image.height() != d->disparity.rows) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    // This part is same as in base class's display... it computes
    // display scaling and vertical/horizontal offsets
    int w = d->disparity.cols;
    int h = d->disparity.rows;

    double scale = qMin((double)width() / w, (double)height() / h);

    w *= scale;
    h *= scale;

    double xd = pos.x() - (width() - w)/2;
    double yd = pos.y() - (height() - h)/2;

    int x = round(xd / scale);
    int y = round(yd / scale);

    if (x >= 0 && y >= 0 && x < d->disparity.cols && y < d->disparity.rows) {
        switch (d->disparity.type()) {
            case CV_8U: {
                return d->disparity.at<unsigned char>(y, x);
            }
            case CV_16S: {
                return d->disparity.at<short>(y, x);
            }
            case CV_32F: {
                return d->disparity.at<float>(y, x);
            }
            default: {
                qWarning() << "Unhandled disparity type:" << d->disparity.type() << "!";
                return std::numeric_limits<float>::quiet_NaN();
            }
        }
    }

    return std::numeric_limits<float>::quiet_NaN();
}


} // Widgets
} // StereoToolbox
} // MVL
