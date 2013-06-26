#include "ReprojectedImageDisplayWidget.h"


ReprojectedImageDisplayWidget::ReprojectedImageDisplayWidget (const QString &t, QWidget *parent)
    : ImageDisplayWidget(t, parent)
{
    setMouseTracking(true); // Enable mouse tracking
}

ReprojectedImageDisplayWidget::~ReprojectedImageDisplayWidget ()
{
    
}

void ReprojectedImageDisplayWidget::setImage (const cv::Mat &newImage, const cv::Mat &newReprojectedPoints)
{
    // Set base image
    ImageDisplayWidget::setImage(newImage);

    reprojectedPoints = newReprojectedPoints;
}


// *********************************************************************
// *             Reprojected coordinates display in tooltip            *
// *********************************************************************
bool ReprojectedImageDisplayWidget::event (QEvent *event)
{
    // Override handling of tool-tip events
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QVector3D coordinates = getCoordinatesAtPixel(helpEvent->pos());

        if (!coordinates.isNull()) {
            QToolTip::showText(helpEvent->globalPos(), QString("%1, %2, %3").arg(coordinates.x()/1000, 0, 'f', 2).arg(coordinates.y()/1000, 0, 'f', 2).arg(coordinates.z()/1000, 0, 'f', 2));
        } else {
            QToolTip::hideText();
            event->ignore();
        }
        
        return true;
    }

    // Chain up to parent
    return ImageDisplayWidget::event(event);
}


// Display image
QVector3D ReprojectedImageDisplayWidget::getCoordinatesAtPixel (const QPoint &pos)
{
    // Make sure there is image displayed
    if (image.isNull()) {
        return QVector3D();
    }

    // Make sure we have reprojected points matrix set, and that is has
    // same dimensions as displayed image
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
