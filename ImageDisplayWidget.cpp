#include "ImageDisplayWidget.h"

ImageDisplayWidget::ImageDisplayWidget (const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setToolTip(text);

    setFrameStyle(QFrame::Box | QFrame::Sunken);
    setAlignment(Qt::AlignCenter);

    setMinimumSize(384, 288);
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


void ImageDisplayWidget::setImage (const cv::Mat &image)
{
    // Convert cv::Mat to QImage
    QImage tmp = cvMatToQImage(image);

    // Create pixmap and store it
    displayPixmap = QPixmap::fromImage(tmp);

    // Display pixmap
    displayScaledPixmap();
}


void ImageDisplayWidget::resizeEvent (QResizeEvent *event)
{
    // Rescale pixmap and display it
    displayScaledPixmap();
}



void ImageDisplayWidget::displayScaledPixmap ()
{
    if (!displayPixmap.isNull()) {
        // Scale pixmap to fit the widget, while preserving aspect ratio
        QPixmap tmp = displayPixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Set scaled pixmap
        setPixmap(tmp);
    } else {
        setPixmap(QPixmap());
    }
}
