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
 
#ifndef IMAGE_DISPLAY_WIDGET_H
#define IMAGE_DISPLAY_WIDGET_H

#include <QtCore>
#include <QtGui>
#include <opencv2/core/core.hpp>


class ImageDisplayWidget : public QFrame
{
    Q_OBJECT

public:
    ImageDisplayWidget (const QString & = QString(), QWidget * = 0);
    virtual ~ImageDisplayWidget ();

    void setImage (const cv::Mat &);
    void setText (const QString &);

    static QImage convertCvMatToQImage (const cv::Mat &src);

protected:
    virtual void paintEvent (QPaintEvent *);

protected:
    QString text;
    QImage image;
};

#endif
