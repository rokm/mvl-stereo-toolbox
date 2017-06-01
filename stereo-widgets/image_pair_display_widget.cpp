/*
 * Stereo Widgets: image pair display widget
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

#include "image_pair_display_widget.h"
#include "image_pair_display_widget_p.h"

#include <opencv2/imgproc.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


ImagePairDisplayWidgetPrivate::ImagePairDisplayWidgetPrivate (ImagePairDisplayWidget *parent)
    : ImageDisplayWidgetPrivate(parent), displayPair(true)
{
}


ImagePairDisplayWidget::ImagePairDisplayWidget (const QString &text, QWidget *parent)
    : ImageDisplayWidget(new ImagePairDisplayWidgetPrivate(this), text, parent)
{
    Q_D(ImagePairDisplayWidget);
}

ImagePairDisplayWidget::~ImagePairDisplayWidget ()
{
}


void ImagePairDisplayWidget::setImagePairROI (const cv::Rect &roiLeft, const cv::Rect &roiRight)
{
    Q_D(ImagePairDisplayWidget);

    // Store ROI
    d->roiLeft = roiLeft;
    d->roiRight = roiRight;

    // Refresh
    update();
}


void ImagePairDisplayWidget::setImage (const cv::Mat &image)
{
    Q_D(ImagePairDisplayWidget);

    d->displayPair = false;

    ImageDisplayWidget::setImage(image);
}


void ImagePairDisplayWidget::setImagePair (const cv::Mat &left, const cv::Mat &right)
{
    Q_D(ImagePairDisplayWidget);

    d->displayPair = true;

    // Make a copy, and mark for the update
    if (left.empty()) {
        d->imageLeft = cv::Mat();
    } else if (left.channels() == 1) {
        cv::cvtColor(left, d->imageLeft, cv::COLOR_GRAY2RGB);
    } else {
        cv::cvtColor(left, d->imageLeft, cv::COLOR_BGR2RGB);
    }
    
    if (right.empty()) {
        d->imageRight = cv::Mat();
    } else if (right.channels() == 1) {
        cv::cvtColor(right, d->imageRight, cv::COLOR_GRAY2RGB);
    } else {
        cv::cvtColor(right, d->imageRight, cv::COLOR_BGR2RGB);
    }

    // Refresh
    update();
}

void ImagePairDisplayWidget::paintEvent (QPaintEvent *event)
{
    Q_D(ImagePairDisplayWidget);

    if (!d->displayPair) {
        return ImageDisplayWidget::paintEvent(event);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect area(0, 0, width(), height());

    // Fill area
    painter.fillRect(area, QBrush(QColor(0, 0, 0, 32), Qt::DiagCrossPattern));

    if (d->imageLeft.empty() || d->imageRight.empty()) {
        // Display text
        painter.drawText(area, Qt::AlignCenter, d->text);
    } else {
        // Display image
        int w = d->imageLeft.cols + d->imageRight.cols;
        int h = qMax(d->imageLeft.rows, d->imageRight.rows);

        int iw, ih;

        double scale = qMin((double)width() / w, (double)height() / h);

        w *= scale;
        h *= scale;

        // *** Draw images ***
        // Move to left image
        painter.translate((width() - w)/2, (height() - h)/2);

        // Left image
        iw = d->imageLeft.cols * scale;
        ih = d->imageLeft.rows * scale;

        painter.drawImage(QRect(0, (h - ih)/2, iw, ih), QImage(d->imageLeft.data, d->imageLeft.cols, d->imageLeft.rows, d->imageLeft.step, QImage::Format_RGB888));

        if ((d->roiLeft.width && d->roiLeft.height) && (d->roiLeft.width != d->imageLeft.cols || d->roiLeft.height != d->imageLeft.rows)) {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawRect(d->roiLeft.x*scale, d->roiLeft.y*scale, d->roiLeft.width*scale, d->roiLeft.height*scale);
        }

        // Move to right image
        painter.translate(iw, 0);

        // Right image
        iw = d->imageRight.cols * scale;
        ih = d->imageRight.rows * scale;

        painter.drawImage(QRect(0, (h - ih)/2, iw, ih), QImage(d->imageRight.data, d->imageRight.cols, d->imageRight.rows, d->imageRight.step, QImage::Format_RGB888));

        if ((d->roiRight.width && d->roiRight.height) && (d->roiRight.width != d->imageRight.cols || d->roiRight.height != d->imageRight.rows)) {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawRect(d->roiRight.x*scale, d->roiRight.y*scale, d->roiRight.width*scale, d->roiRight.height*scale);
        }

        // Draw horizontal lines
        painter.resetTransform();
        painter.translate((width() - w)/2, (height() - h)/2);

        int maxHeight = qMax(d->imageLeft.rows, d->imageRight.rows);
        int numColors = 4, c = 0; // Pen color counter
        for (int i = 0; i < maxHeight; i += 16) {
            painter.setPen(QPen(QColor(0, 255*(c+1)/numColors, 0, 255), 1.5));

            painter.drawLine(0, i*scale, w, i*scale);

            // Alternate the pen colors
            c = (c + 1) % numColors;
        }

    }

    // Draw frame on top of it all
    QFrame::paintEvent(event);
}


} // Widgets
} // StereoToolbox
} // MVL
