#ifndef STEREO_PIPELINE_H
#define STEREO_PIPELINE_H

#include <QtCore>

#include <opencv2/core/core.hpp>


class StereoMethod;

class StereoPipeline : public QObject
{
    Q_OBJECT
    
public:
    StereoPipeline (QObject * = 0);
    virtual ~StereoPipeline ();

    void setCalibration (const QString &, const QString &);

    void setStereoMethod (StereoMethod *);

    void processImagePair (const cv::Mat &, const cv::Mat &);

protected:
    // Stereo method
    StereoMethod *method;
    
    // Cached input images
    cv::Mat inputImageL;
    cv::Mat inputImageR;

    // Cached rectified input images
    cv::Mat rectifiedImageL;
    cv::Mat rectifiedImageR;

    // Cached depth image
    cv::Mat depthImage;
};

#endif
