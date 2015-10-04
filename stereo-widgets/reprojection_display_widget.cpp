/*
 * Stereo Widgets: reprojection display widget
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

#include "reprojection_display_widget.h"
#include "reprojection_display_widget_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


ReprojectionDisplayWidgetPrivate::ReprojectionDisplayWidgetPrivate (ReprojectionDisplayWidget *parent)
    : ImageDisplayWidgetPrivate(parent)
{
}


ReprojectionDisplayWidget::ReprojectionDisplayWidget (const QString &text, QWidget *parent)
    : ImageDisplayWidget(new ReprojectionDisplayWidgetPrivate(this), text, parent)
{
    setMouseTracking(true); // Enable mouse tracking
}

ReprojectionDisplayWidget::~ReprojectionDisplayWidget ()
{
}


void ReprojectionDisplayWidget::mouseMoveEvent (QMouseEvent *event)
{
    emit coordinatesUnderMouseChanged(getCoordinatesAtPixel(event->pos()));

    // Chain up to parent
    return ImageDisplayWidget::mouseMoveEvent(event);
}

void ReprojectionDisplayWidget::setPoints (const cv::Mat &reprojectedPoints)
{
    Q_D(ReprojectionDisplayWidget);

    d->reprojectedPoints = reprojectedPoints;
    emit coordinatesUnderMouseChanged(getCoordinatesAtPixel(mapFromGlobal(QCursor::pos())));
}

// Display image
QVector3D ReprojectionDisplayWidget::getCoordinatesAtPixel (const QPoint &pos)
{
    Q_D(ReprojectionDisplayWidget);

    // Make sure there is image displayed
    if (d->image.isNull()) {
        return QVector3D();
    }

    // Validate dimensions
    if (d->image.width() != d->reprojectedPoints.cols || d->image.height() != d->reprojectedPoints.rows) {
        return QVector3D();
    }

    // This part is same as in base class's display... it computes
    // display scaling and vertical/horizontal offsets
    int w = d->image.width();
    int h = d->image.height();

    double scale = qMin((double)width() / w, (double)height() / h);

    w *= scale;
    h *= scale;

    double xd = pos.x() - (width() - w)/2;
    double yd = pos.y() - (height() - h)/2;

    int x = round(xd / scale);
    int y = round(yd / scale);

    if (x >= 0 && y >= 0 && x < d->reprojectedPoints.cols && y < d->reprojectedPoints.rows) {
        const cv::Vec3f &entry = d->reprojectedPoints.at<cv::Vec3f>(y, x);
        return QVector3D(entry[0], entry[1], entry[2]);
    }

    return QVector3D();
}


} // Widgets
} // StereoToolbox
} // MVL
