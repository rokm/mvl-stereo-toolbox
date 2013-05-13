#ifndef STEREO_METHOD
#define STEREO_METHOD

#include <QtCore>

#include <opencv2/core/core.hpp>


class StereoMethod : public QObject
{
    Q_OBJECT

public:
    StereoMethod (QObject * = 0);
    virtual ~StereoMethod ();

    virtual const cv::Mat &processImagePair (const cv::Mat &, const cv::Mat &) = 0;
};

#endif
