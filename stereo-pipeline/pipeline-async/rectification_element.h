#ifndef MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__RECTIFICATION_ELEMENT_H
#define MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__RECTIFICATION_ELEMENT_H


#include "element.h"

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

class Rectification;

namespace AsyncPipeline {


class RectificationElement : public Element
{
    Q_OBJECT

public:
    RectificationElement (QObject *parent = nullptr);
    virtual ~RectificationElement ();

    Rectification *getRectification ();

    void rectifyImages (const cv::Mat &imageL, const cv::Mat &imageR);

    cv::Mat getLeftImage () const;
    cv::Mat getRightImage () const;

    void getImages (cv::Mat &imageLeft, cv::Mat &imageRight) const;

    cv::Mat getReprojectionMatrix () const;

signals:
    void eject ();
    void imageRectificationRequest (const cv::Mat imageL, const cv::Mat imageR);

    void imagesChanged ();

    void calibrationChanged (bool valid);
    void performRectificationChanged (bool enabled);

protected:
    // Rectification object
    Rectification *rectification;

    mutable QMutex mutex; // Method mutex


    // Cached input images
    mutable QReadWriteLock lock;

    cv::Mat imageL;
    cv::Mat imageR;

    // Worker thread's local variables
    struct {
        QTime timer;
        cv::Mat imageL;
        cv::Mat imageR;
        int processingTime;
    } threadData;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
