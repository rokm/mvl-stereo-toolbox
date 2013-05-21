#ifndef IMAGE_PAIR_DISPLAY_WIDGET
#define IMAGE_PAIR_DISPLAY_WIDGET

#include "ImageDisplayWidget.h"


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

#endif
