/*
 * MVL Stereo Toolbox: OpenCV image display widgets
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


// *********************************************************************
// *                       Image display widget                        *
// *********************************************************************
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


// *********************************************************************
// *                    Image pair display widget                      *
// *********************************************************************
class ImagePairDisplayWidget : public ImageDisplayWidget
{
    Q_OBJECT

public:
    ImagePairDisplayWidget (const QString & = QString(), QWidget * = 0);
    virtual ~ImagePairDisplayWidget ();

    void setImagePairROI (const cv::Rect &, const cv::Rect &);
    void setImagePair (const cv::Mat &, const cv::Mat &);

protected:
    virtual void paintEvent (QPaintEvent *);

protected:
    cv::Rect roiLeft;
    cv::Rect roiRight;
    
    QImage imageLeft;
    QImage imageRight;
};


// *********************************************************************
// *                Calibration pattern display widget                 *
// *********************************************************************
class CalibrationPatternDisplayWidget : public ImageDisplayWidget
{
    Q_OBJECT

public:
    CalibrationPatternDisplayWidget (const QString & = QString(), QWidget * = 0);
    virtual ~CalibrationPatternDisplayWidget ();

    void setPattern (bool, const std::vector<cv::Point2f> &, const cv::Size &);

protected:
    virtual void paintEvent (QPaintEvent *);

protected:
    bool patternFound;
    cv::Size patternSize;
    std::vector<cv::Point2f> patternPoints;
};


// *********************************************************************
// *                 Reprojected image display widget                  *
// *********************************************************************
class ReprojectedImageDisplayWidget : public ImageDisplayWidget
{
    Q_OBJECT

public:
    ReprojectedImageDisplayWidget (const QString & = QString(), QWidget * = 0);
    virtual ~ReprojectedImageDisplayWidget ();

    void setImage (const cv::Mat &, const cv::Mat &);

protected:
    virtual bool event (QEvent *);

    QVector3D getCoordinatesAtPixel (const QPoint &);

protected:
    cv::Mat reprojectedPoints;
};


#endif
