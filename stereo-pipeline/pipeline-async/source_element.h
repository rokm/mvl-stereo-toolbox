#ifndef MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__SOURCE_ELEMENT_H
#define MVL_STEREO_TOOLBOX__PIPELINE_ASYNC__SOURCE_ELEMENT_H


#include "element.h"

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {

class ImagePairSource;

namespace AsyncPipeline {


class SourceElement : public Element
{
    Q_OBJECT

public:
    SourceElement (QObject *parent = nullptr);
    virtual ~SourceElement ();

    void setImagePairSource (QObject *source);
    QObject *getImagePairSource ();

    cv::Mat getLeftImage () const;
    cv::Mat getRightImage () const;

    void getImages (cv::Mat imageLeft, cv::Mat imageRight) const;

signals:
    void eject ();

    void imagesChanged (cv::Mat left, cv::Mat right);

protected:
    // Image pair source object
    QObject *sourceObject;
    QObject *sourceParent;
    ImagePairSource *sourceIface;

    QList<QMetaObject::Connection> signalConnections;

    // Cached input images
    mutable QReadWriteLock lock;

    cv::Mat imageL;
    cv::Mat imageR;
};


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL


#endif
