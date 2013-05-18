#ifndef IMAGE_SOURCE_H
#define IMAGE_SOURCE_H

#include <QtCore>

#include <opencv2/core/core.hpp>


class ImageSource : public QObject
{
    Q_OBJECT

public:
    ImageSource (QObject * = 0);
    virtual ~ImageSource ();

    virtual void getImages (cv::Mat &, cv::Mat &);

    // Config interface
    const QString &getShortName () const;
    QWidget *getConfigWidget ();

signals:
    void imagesChanged ();

protected:
    QString shortName;
    QWidget *configWidget;

    // Images
    QReadWriteLock imagesLock;
    
    cv::Mat imageLeft;
    cv::Mat imageRight;
};

#endif
