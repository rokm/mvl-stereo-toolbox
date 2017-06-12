#ifndef MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__VISUALIZATION_ELEMENT_H
#define MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__VISUALIZATION_ELEMENT_H


#include "element.h"

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

class DisparityVisualization;

namespace AsyncPipeline {


class VisualizationElement : public Element
{
    Q_OBJECT

public:
    VisualizationElement (QObject *parent = nullptr);
    virtual ~VisualizationElement ();

    DisparityVisualization *getVisualization ();

    void visualizeDisparity (const cv::Mat &disparity, int numDisparityLevels);

    cv::Mat getImage () const;
    void getImage (cv::Mat &image) const;

signals:
    void eject ();
    void disparityVisualizationRequest (const cv::Mat disparity, int numDisparityLevels);

    void visualizationMethodChanged ();
    void imageChanged ();

protected:
    // Visualization object
    DisparityVisualization *visualization;

    mutable QMutex mutex; // Method mutex


    // Cached input images
    mutable QReadWriteLock lock;

    cv::Mat image;

    // Worker thread's local variables
    struct {
        QTime timer;
        cv::Mat image;
        int processingTime;
    } threadData;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
