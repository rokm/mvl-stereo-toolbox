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

#include "image_display_widget.h"
#include "image_display_widget_p.h"

#include <opencv2/imgproc.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


ImageDisplayWidgetPrivate::ImageDisplayWidgetPrivate (ImageDisplayWidget *parent)
    : q_ptr(parent)
{
}



ImageDisplayWidget::ImageDisplayWidget (const QString &text, QWidget *parent)
    : QFrame(parent), d_ptr(new ImageDisplayWidgetPrivate(this))
{
    setFrameStyle(QFrame::Box | QFrame::Sunken);
    setLineWidth(2);
}

ImageDisplayWidget::ImageDisplayWidget (ImageDisplayWidgetPrivate *d, const QString &text, QWidget *parent)
    : QFrame(parent), d_ptr(d)
{
    setFrameStyle(QFrame::Box | QFrame::Sunken);
    setLineWidth(2);
}

ImageDisplayWidget::~ImageDisplayWidget ()
{
}


void ImageDisplayWidget::setImage (const cv::Mat &image)
{
    Q_D(ImageDisplayWidget);

    // Make a copy, and mark for the update
    if (image.channels() == 1) {
        cv::cvtColor(image, d->image, cv::COLOR_GRAY2RGB);
    } else {
        cv::cvtColor(image, d->image, cv::COLOR_BGR2RGB);
    }

    // Refresh
    update();
}

void ImageDisplayWidget::setText (const QString &text)
{
    Q_D(ImageDisplayWidget);

    // Store new text
    d->text = text;

    // Refresh
    update();
}


void ImageDisplayWidget::paintEvent (QPaintEvent *event)
{
    Q_D(ImageDisplayWidget);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect area(0, 0, width(), height());

    // Fill area
    painter.fillRect(area, QBrush(QColor(0, 0, 0, 32), Qt::DiagCrossPattern));

    if (d->image.empty()) {
        // Display text
        painter.drawText(area, Qt::AlignCenter, d->text);
    } else {
        // Display image
        int w = d->image.cols;
        int h = d->image.rows;

        double scale = qMin((double)width() / w, (double)height() / h);

        w *= scale;
        h *= scale;

        painter.translate((width() - w)/2, (height() - h)/2);

        painter.drawImage(QRect(0, 0, w, h), QImage(d->image.data, d->image.cols, d->image.rows, d->image.step, QImage::Format_RGB888));
    }

    // Draw frame on top of it all
    QFrame::paintEvent(event);
}


// *********************************************************************
// *                   OpenCV to Qt image conversion                   *
// *********************************************************************
QImage ImageDisplayWidget::convertCvMatToQImage (const cv::Mat &src)
{
    QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
    QRgb *destrow;
    int x, y;

    if (src.channels() == 1) {
        // Gray
        for (int y = 0; y < src.rows; ++y) {
            const unsigned char *srcrow = src.ptr<unsigned char>(y);
            destrow = (QRgb*)dest.scanLine(y);
            for (int x = 0; x < src.cols; ++x) {
                destrow[x] = qRgba(srcrow[x], srcrow[x], srcrow[x], 255);
            }
        }
    } else if (src.channels() == 3) {
        // BGR
        for (y = 0; y < src.rows; ++y) {
            const cv::Vec3b *srcrow = src.ptr<cv::Vec3b>(y);
            destrow = (QRgb*)dest.scanLine(y);
            for (x = 0; x < src.cols; ++x) {
                destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
            }
        }
    } else if (src.channels() == 4) {
        // BGRA
        for (y = 0; y < src.rows; ++y) {
            const cv::Vec4b *srcrow = src.ptr<cv::Vec4b>(y);
            destrow = (QRgb*)dest.scanLine(y);
            for (x = 0; x < src.cols; ++x) {
                destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], srcrow[x][3]);
            }
        }
    }

    return dest;
}


} // Widgets
} // StereoToolbox
} // MVL
