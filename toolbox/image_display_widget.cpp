/*
 * MVL Stereo Toolbox: OpenCV image display widgets
 * Copyright (C) 2013 Rok Mandeljc
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

#ifdef _MSC_VER
#define round(x) ((x) >= 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))
#define NAN std::numeric_limits<float>::quiet_NaN()
#endif


// *********************************************************************
// *                        Image display widget                       *
// *********************************************************************
ImageDisplayWidget::ImageDisplayWidget (const QString &t, QWidget *parent)
    : QFrame(parent), text(t)
{
    setFrameStyle(QFrame::Box | QFrame::Sunken);
    setLineWidth(2);
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

void ImageDisplayWidget::setText (const QString &newText)
{
    // Store new text
    text = newText;

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
        
        painter.translate((width() - w)/2, (height() - h)/2);
        painter.drawImage(QRect(0, 0, w, h), image);
    }

    // Draw frame on top of it all
    QFrame::paintEvent(event);
}


// *********************************************************************
// *                    Image pair display widget                      *
// *********************************************************************
ImagePairDisplayWidget::ImagePairDisplayWidget (const QString &t, QWidget *parent)
    : ImageDisplayWidget(t, parent)
{
}

ImagePairDisplayWidget::~ImagePairDisplayWidget ()
{
}


void ImagePairDisplayWidget::setImagePairROI (const cv::Rect &left, const cv::Rect &right)
{
    // Store ROI
    roiLeft = left;
    roiRight = right;

    // Refresh
    update();
}

void ImagePairDisplayWidget::setImagePair (const cv::Mat &left, const cv::Mat &right)
{
    // Convert cv::Mat to QImage
    imageLeft = convertCvMatToQImage(left);
    imageRight = convertCvMatToQImage(right);

    // Refresh
    update();
}

void ImagePairDisplayWidget::paintEvent (QPaintEvent *event)
{    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect area(0, 0, width(), height());

    // Fill area
    painter.fillRect(area, QBrush(QColor(0, 0, 0, 32), Qt::DiagCrossPattern));

    if (imageLeft.isNull() || imageRight.isNull()) {
        // Display text
        painter.drawText(area, Qt::AlignCenter, text);
    } else {
        // Display image        
        int w = imageLeft.width() + imageRight.width();
        int h = qMax(imageLeft.height(), imageRight.height());

        int iw, ih;
        
        double scale = qMin((double)width() / w, (double)height() / h);

        w *= scale;
        h *= scale;

        // *** Draw images ***        
        // Move to left image
        painter.translate((width() - w)/2, (height() - h)/2);

        // Left image
        iw = imageLeft.width() * scale;
        ih = imageLeft.height() * scale;
        
        painter.drawImage(QRect(0, (h - ih)/2, iw, ih), imageLeft);

        if ((roiLeft.width && roiLeft.height) && (roiLeft.width != imageLeft.width() || roiLeft.height != imageLeft.height())) {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawRect(roiLeft.x*scale, roiLeft.y*scale, roiLeft.width*scale, roiLeft.height*scale);
        }
        
        // Move to right image
        painter.translate(iw, 0);

        // Right image
        iw = imageRight.width() * scale;
        ih = imageRight.height() * scale;
        
        painter.drawImage(QRect(0, (h - ih)/2, iw, ih), imageRight);

        if ((roiRight.width && roiRight.height) && (roiRight.width != imageRight.width() || roiRight.height != imageRight.height())) {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawRect(roiRight.x*scale, roiRight.y*scale, roiRight.width*scale, roiRight.height*scale);
        }

        // Draw horizontal lines        
        painter.resetTransform();
        painter.translate((width() - w)/2, (height() - h)/2);

        int maxHeight = qMax(imageLeft.height(), imageRight.height());
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


// *********************************************************************
// *                Calibration pattern display widget                 *
// *********************************************************************
CalibrationPatternDisplayWidget::CalibrationPatternDisplayWidget (const QString &t, QWidget *parent)
    : ImageDisplayWidget(t, parent)
{
}

CalibrationPatternDisplayWidget::~CalibrationPatternDisplayWidget ()
{
}

void CalibrationPatternDisplayWidget::setPattern (bool found, const std::vector<cv::Point2f> &points, const cv::Size &size)
{
    // Store pattern
    patternSize = size;
    patternFound = found;
    patternPoints = points;

    // Refresh
    update();
}

void CalibrationPatternDisplayWidget::paintEvent (QPaintEvent *event)
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

        painter.translate((width() - w)/2, (height() - h)/2);

        painter.drawImage(QRect(0, 0, w, h), image);

        // Display pattern
        painter.scale(scale, scale);

        int r = 8;
        QColor color;
        QPointF currentPoint, previousPoint;

        QPointF lineOffsetA1 = QPointF(-r,-r) / sqrtf(2.0);
        QPointF lineOffsetA2 = QPointF(r,r) / sqrtf(2.0);
        QPointF lineOffsetB1 = QPointF(-r,r) / sqrtf(2.0);
        QPointF lineOffsetB2 = QPointF(r,-r) / sqrtf(2.0);
        
        for (unsigned int i = 0; i < patternPoints.size(); i++) {            
            // Select color based on which row we are drawing
            if (patternFound) {
                int row = i / patternSize.width;
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
            currentPoint = QPointF(patternPoints[i].x, patternPoints[i].y);

            // Draw circle
            painter.setPen(QPen(color, 2.0));
            painter.drawEllipse(currentPoint, r, r);

            // Draw crossed lines
            painter.drawLine(currentPoint + lineOffsetA1, currentPoint + lineOffsetA2);
            painter.drawLine(currentPoint + lineOffsetB1, currentPoint + lineOffsetB2);
            
            // Draw connecting line; only if pattern has been found
            if (!patternFound) {
                continue;
            }
            
            if (i > 0) {
                if (i % patternSize.width) {
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


// *********************************************************************
// *                  Disparity image display widget                   *
// *********************************************************************
DisparityImageDisplayWidget::DisparityImageDisplayWidget (const QString &t, QWidget *parent)
    : ImageDisplayWidget(t, parent)
{    
    setMouseTracking(true); // Enable mouse tracking
}

DisparityImageDisplayWidget::~DisparityImageDisplayWidget ()
{
}

void DisparityImageDisplayWidget::mouseMoveEvent (QMouseEvent *event)
{
    emit disparityUnderMouseChanged(getDisparityAtPixel(event->pos()));
    
    // Chain up to parent
    return ImageDisplayWidget::mouseMoveEvent(event);
}


void DisparityImageDisplayWidget::setDisparity (const cv::Mat &newDisparity)
{
    disparity = newDisparity;
    emit disparityUnderMouseChanged(getDisparityAtPixel(mapFromGlobal(QCursor::pos())));
}

float DisparityImageDisplayWidget::getDisparityAtPixel (const QPoint &pos)
{
    // Make sure there is image displayed
    if (!disparity.data) {
        return NAN;
    }

    // Validate dimensions
    if (image.width() != disparity.cols || image.height() != disparity.rows) {
        return NAN;
    }
    
    // This part is same as in base class's display... it computes
    // display scaling and vertical/horizontal offsets
    int w = disparity.cols;
    int h = disparity.rows;
        
    double scale = qMin((double)width() / w, (double)height() / h);

    w *= scale;
    h *= scale;

    double xd = pos.x() - (width() - w)/2;
    double yd = pos.y() - (height() - h)/2;

    int x = round(xd / scale);
    int y = round(yd / scale);

    if (x >= 0 && y >= 0 && x < disparity.cols && y < disparity.rows) {
        switch (disparity.type()) {
            case CV_8U: {
                return disparity.at<unsigned char>(y, x);
            }
            case CV_32F: {
                return disparity.at<float>(y, x);
            }
            default: {
                qWarning() << "Unhandled disparity type:" << disparity.type() << "!";
                return NAN;
            }
        }
    }

    return NAN;
}


// *********************************************************************
// *                 Reprojected image display widget                  *
// *********************************************************************
ReprojectedImageDisplayWidget::ReprojectedImageDisplayWidget (const QString &t, QWidget *parent)
    : ImageDisplayWidget(t, parent)
{
    setMouseTracking(true); // Enable mouse tracking
}

ReprojectedImageDisplayWidget::~ReprojectedImageDisplayWidget ()
{
    
}

void ReprojectedImageDisplayWidget::mouseMoveEvent (QMouseEvent *event)
{
    emit coordinatesUnderMouseChanged(getCoordinatesAtPixel(event->pos()));
    
    // Chain up to parent
    return ImageDisplayWidget::mouseMoveEvent(event);
}

void ReprojectedImageDisplayWidget::setPoints (const cv::Mat &newReprojectedPoints)
{
    reprojectedPoints = newReprojectedPoints;
    emit coordinatesUnderMouseChanged(getCoordinatesAtPixel(mapFromGlobal(QCursor::pos())));
}

// Display image
QVector3D ReprojectedImageDisplayWidget::getCoordinatesAtPixel (const QPoint &pos)
{
    // Make sure there is image displayed
    if (image.isNull()) {
        return QVector3D();
    }

    // Validate dimensions
    if (image.width() != reprojectedPoints.cols || image.height() != reprojectedPoints.rows) {
        return QVector3D();
    }
    
    // This part is same as in base class's display... it computes
    // display scaling and vertical/horizontal offsets
    int w = image.width();
    int h = image.height();
        
    double scale = qMin((double)width() / w, (double)height() / h);

    w *= scale;
    h *= scale;

    double xd = pos.x() - (width() - w)/2;
    double yd = pos.y() - (height() - h)/2;

    int x = round(xd / scale);
    int y = round(yd / scale);

    if (x >= 0 && y >= 0 && x < reprojectedPoints.cols && y < reprojectedPoints.rows) {
        const cv::Vec3f &entry = reprojectedPoints.at<cv::Vec3f>(y, x);
        return QVector3D(entry[0], entry[1], entry[2]);
    }

    return QVector3D();
}
