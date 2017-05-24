#ifndef MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__REPROJECTION_ELEMENT_H
#define MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__REPROJECTION_ELEMENT_H


#include "element.h"

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

class Reprojection;

namespace AsyncPipeline {


class ReprojectionElement : public Element
{
    Q_OBJECT

public:
    ReprojectionElement (QObject *parent = nullptr);
    virtual ~ReprojectionElement ();

    Reprojection *getReprojection ();

    void reprojectDisparity (const cv::Mat &disparity, int numDisparityLevels);

    cv::Mat getPoints () const;

signals:
    void eject ();
    void reprojectionRequest (cv::Mat disparity);

    void pointsChanged (cv::Mat points);

protected:
    // Reprojection object
    Reprojection *reprojection;

    mutable QMutex mutex; // Method mutex


    // Cached input images
    mutable QReadWriteLock lock;

    cv::Mat points;

    // Worker thread's local variables
    struct {
        QTime timer;
        cv::Mat points;
        int processingTime;
    } threadData;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
