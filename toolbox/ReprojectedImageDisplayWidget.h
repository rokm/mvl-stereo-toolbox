#ifndef REPROJECTED_IMAGE_DISPLAY_WIDGET_H
#define REPROJECTED_IMAGE_DISPLAY_WIDGET_H

#include "ImageDisplayWidget.h"


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
