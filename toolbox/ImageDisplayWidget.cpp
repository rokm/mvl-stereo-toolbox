/*
 * MVL Stereo Toolbox: OpenCV image display widget
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include "ImageDisplayWidget.h"


ImageDisplayWidget::ImageDisplayWidget (const QString &t, QWidget *parent)
    : QFrame(parent), text(t)
{
    setToolTip(text);

    setFrameStyle(QFrame::Box | QFrame::Sunken);
    setLineWidth(2);
    
    //setMinimumSize(384, 288);
}

ImageDisplayWidget::~ImageDisplayWidget ()
{
}

QImage ImageDisplayWidget::convertCvMatToQImage (const cv::Mat &src)
{
    QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
    QRgb *destrow;
    int x, y;

    if (src.channels() ==1) {
        // Gray
        for (int y = 0; y < src.rows; ++y) {
            const unsigned char *srcrow = src.ptr<unsigned char>(y);
            destrow = (QRgb*)dest.scanLine(y);
            for (int x = 0; x < src.cols; ++x) {
                destrow[x] = qRgba(srcrow[x], srcrow[x], srcrow[x], 255);
            }
        }
    } else if (src.channels() == 3) {
        // RGB
        for (y = 0; y < src.rows; ++y) {
            const cv::Vec3b *srcrow = src.ptr<cv::Vec3b>(y);
            destrow = (QRgb*)dest.scanLine(y);
            for (x = 0; x < src.cols; ++x) {
                destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
            }
        }
    } else if (src.channels() == 4) {
        // RGBA
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


void ImageDisplayWidget::setImage (const cv::Mat &img)
{
    // Convert cv::Mat to QImage
    image = convertCvMatToQImage(img);

    // Refresh
    update();
}

void ImageDisplayWidget::paintEvent (QPaintEvent *event)
{    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect area(0, 0, width(), height());

    // Fill area
    painter.fillRect(area, QBrush(QColor(0, 0, 0, 32), Qt::DiagCrossPattern));

    if (image.isNull()) {
        // Display text
        painter.drawText(area, Qt::AlignCenter, text);
    } else {
        // Display image        
        int w = image.width();
        int h = image.height();
        
        double scale = qMin((double)width() / w, (double)height() / h);

        w *= scale;
        h *= scale;
            
        painter.drawImage(QRect((width() - w)/2, (height() - h)/2, w, h), image);
    }

    // Draw frame on top of it all
    QFrame::paintEvent(event);
}
