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


static inline QImage cvMatToQImage (const cv::Mat &src)
{
    QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
    QRgb *destrow;
    int x, y;
    
    if (src.channels() == 3) {
        // RGB
        for (y = 0; y < src.rows; ++y) {
            const cv::Vec3b *srcrow = src.ptr<cv::Vec3b>(y);
            destrow = (QRgb*)dest.scanLine(y);
            for (x = 0; x < src.cols; ++x) {
                destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
            }
        }
    } else {
        // Gray
        for (int y = 0; y < src.rows; ++y) {
            const unsigned char *srcrow = src.ptr<unsigned char>(y);
            destrow = (QRgb*)dest.scanLine(y);
            for (int x = 0; x < src.cols; ++x) {
                destrow[x] = qRgba(srcrow[x], srcrow[x], srcrow[x], 255);
            }
        }
    }
    
    return dest;
}


void ImageDisplayWidget::setImage (const cv::Mat &img)
{
    // Convert cv::Mat to QImage
    image = cvMatToQImage(img);

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
