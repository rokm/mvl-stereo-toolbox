/*
 * Stereo Widgets: calibration pattern display widget
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

#include "calibration_pattern_display_widget.h"
#include "calibration_pattern_display_widget_p.h"


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


CalibrationPatternDisplayWidgetPrivate::CalibrationPatternDisplayWidgetPrivate (CalibrationPatternDisplayWidget *parent)
    : ImageDisplayWidgetPrivate(parent)
{
}


CalibrationPatternDisplayWidget::CalibrationPatternDisplayWidget (const QString &text, QWidget *parent)
    : ImageDisplayWidget(new CalibrationPatternDisplayWidgetPrivate(this), text, parent)
{
}

CalibrationPatternDisplayWidget::~CalibrationPatternDisplayWidget ()
{
}


void CalibrationPatternDisplayWidget::setPattern (bool found, const std::vector<cv::Point2f> &points, const cv::Size &size)
{
    Q_D(CalibrationPatternDisplayWidget);

    // Store pattern
    d->patternSize = size;
    d->patternFound = found;
    d->patternPoints = points;

    // Refresh
    update();
}


void CalibrationPatternDisplayWidget::paintEvent (QPaintEvent *event)
{
    Q_D(CalibrationPatternDisplayWidget);

    if (d->imageChanged) {
        if (d->image.empty()) {
            d->pixmap = QPixmap(); // Clear
        } else {
            d->pixmap = QPixmap::fromImage(convertCvMatToQImage(d->image));
        }
        d->imageChanged = false;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect area(0, 0, width(), height());

    // Fill area
    painter.fillRect(area, QBrush(QColor(0, 0, 0, 32), Qt::DiagCrossPattern));

    if (d->pixmap.isNull()) {
        // Display text
        painter.drawText(area, Qt::AlignCenter, d->text);
    } else {
        // Display image
        int w = d->pixmap.width();
        int h = d->pixmap.height();

        double scale = qMin((double)width() / w, (double)height() / h);

        w *= scale;
        h *= scale;

        painter.translate((width() - w)/2, (height() - h)/2);

        painter.drawPixmap(QRect(0, 0, w, h), d->pixmap);

        // Display pattern
        painter.scale(scale, scale);

        int r = 8;
        QColor color;
        QPointF currentPoint, previousPoint;

        QPointF lineOffsetA1 = QPointF(-r,-r) / sqrtf(2.0);
        QPointF lineOffsetA2 = QPointF(r,r) / sqrtf(2.0);
        QPointF lineOffsetB1 = QPointF(-r,r) / sqrtf(2.0);
        QPointF lineOffsetB2 = QPointF(r,-r) / sqrtf(2.0);

        for (unsigned int i = 0; i < d->patternPoints.size(); i++) {
            // Select color based on which row we are drawing
            if (d->patternFound) {
                int row = i / d->patternSize.width;
                switch (row % 6) {
                    case 0: color = QColor(255,   0,   0); break;
                    case 1: color = QColor(255, 128,   0); break;
                    case 2: color = QColor(200, 200,   0); break;
                    case 3: color = QColor(  0, 255,   0); break;
                    case 4: color = QColor(  0,   0, 255); break;
                    case 5: color = QColor(255,   0, 255); break;
                }
            } else {
                color = Qt::red;
            }

            // Point coordinates
            currentPoint = QPointF(d->patternPoints[i].x, d->patternPoints[i].y);

            // Draw circle
            painter.setPen(QPen(color, 2.0));
            painter.drawEllipse(currentPoint, r, r);

            // Draw crossed lines
            painter.drawLine(currentPoint + lineOffsetA1, currentPoint + lineOffsetA2);
            painter.drawLine(currentPoint + lineOffsetB1, currentPoint + lineOffsetB2);

            // Draw connecting line; only if pattern has been found
            if (!d->patternFound) {
                continue;
            }

            if (i > 0) {
                if (i % d->patternSize.width) {
                    painter.setPen(QPen(color, 2.0, Qt::SolidLine));
                } else {
                    painter.setPen(QPen(color, 2.0, Qt::DashLine));
                }
                painter.drawLine(previousPoint, currentPoint);
            }

            previousPoint = currentPoint;
        }
    }

    // Draw frame on top of it all
    QFrame::paintEvent(event);
}


} // Widgets
} // StereoToolbox
} // MVL
