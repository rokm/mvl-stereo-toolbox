#ifndef STEREO_REPROJECTION_H
#define STEREO_REPROJECTION_H

#include <QtCore>
#include <opencv2/core/core.hpp>


class StereoReprojection : public QObject
{
    Q_OBJECT
    
public:
    StereoReprojection (QObject * = 0);
    ~StereoReprojection ();

    void setUseGpu (bool);
    bool getUseGpu () const;

    void setReprojectionMatrix (const cv::Mat &);
    const cv::Mat &getReprojectionMatrix () const;

    void reprojectStereoDisparity (const cv::Mat &, cv::Mat &) const;

signals:
    void useGpuChanged (bool);
    void reprojectionMatrixChanged ();

protected:
    cv::Mat Q;
    bool useGpu;
};


#endif
