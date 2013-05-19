#ifndef IMAGE_DISPLAY_WIDGET
#define IMAGE_DISPLAY_WIDGET

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

protected:
    virtual void paintEvent (QPaintEvent *);

protected:
    QString text;
    QImage image;
};

#endif
