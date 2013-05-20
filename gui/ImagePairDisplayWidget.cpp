#include "ImagePairDisplayWidget.h"

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

#include <opencv2/highgui/highgui.hpp>
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

        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(roiLeft.x*scale, roiLeft.y*scale, roiLeft.width*scale, roiLeft.height*scale);
        
        // Move to right image
        painter.translate(iw, 0);

        // Right image
        iw = imageRight.width() * scale;
        ih = imageRight.height() * scale;
        
        painter.drawImage(QRect(0, (h - ih)/2, iw, ih), imageRight);

        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(roiRight.x*scale, roiRight.y*scale, roiRight.width*scale, roiRight.height*scale);

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
