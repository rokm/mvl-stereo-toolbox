#ifndef MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__METHOD_ELEMENT_H
#define MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__METHOD_ELEMENT_H


#include "element.h"

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

class StereoMethod;

namespace AsyncPipeline {


class MethodElement : public Element
{
    Q_OBJECT

public:
    MethodElement (QObject *parent = nullptr);
    virtual ~MethodElement ();

    void setStereoMethod (QObject *method);
    QObject *getStereoMethod ();

    void loadParameters (const QString &filename);
    void saveParameters (const QString &filename) const;

    void computeDisparity (const cv::Mat &imageL, const cv::Mat &imageR);

    cv::Mat getDisparity () const;
    void getDisparity (cv::Mat &disparity, int &numDisparityLevels) const;

signals:
    void eject ();
    void methodChanged ();
    void parameterChanged ();

    void disparityComputationRequest (const cv::Mat imageL, const cv::Mat imageR);
    void disparityChanged ();

protected:
    // Stereo method object
    QObject *methodObject;
    QObject *methodParent;
    StereoMethod *methodIface;

    QList<QMetaObject::Connection> signalConnections;

    mutable QMutex mutex; // Method mutex

    // Cached disparity (under parent's lock!)
    cv::Mat disparity;
    int numDisparityLevels;

    // Worker thread's local variables
    struct {
        QTime timer;
        cv::Mat disparity;
        int numDisparityLevels;
        int processingTime;
    } threadData;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
