#include "source_element.h"

#include <stereo-pipeline/image_pair_source.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace AsyncPipeline {


SourceElement::SourceElement (QObject *parent)
    : Element("ImagePairSource", parent),
      sourceObject(nullptr),
      sourceParent(nullptr),
      sourceIface(nullptr)
{
    // Update time and FPS statistics (local loop)
    connect(this, &SourceElement::imagesChanged, this, &SourceElement::incrementUpdateCount);
}

SourceElement::~SourceElement ()
{
    emit ejectSource(); // Eject source
}


void SourceElement::setImagePairSource (QObject *newSource)
{
    QMetaObject::Connection tmpConnection;

    // Try to get the interface
    ImagePairSource *newSourceIface = qobject_cast<ImagePairSource *>(newSource);
    if (!newSourceIface) {
        qWarning() << "Passed object does not implement ImagePairSource interface!";
        return;
    }

    // Eject old source
    emit ejectSource();

    // Insert new source
    sourceObject = newSource;
    sourceIface = newSourceIface;

    sourceParent = sourceObject->parent(); // Store parent
    sourceObject->setParent(nullptr);

    sourceObject->moveToThread(thread);

    tmpConnection = connect(sourceObject, SIGNAL(imagesChanged(cv::Mat, cv::Mat)), this, SIGNAL(imagesChanged(cv::Mat, cv::Mat)), Qt::QueuedConnection); // Must use old syntax because signal is defined in interface!
    signalConnections.append(tmpConnection);

    tmpConnection = connect(sourceObject, SIGNAL(error(QString)), this, SIGNAL(error(QString)), Qt::QueuedConnection); // Must use old syntax because signal is defined in interface!
    signalConnections.append(tmpConnection);

    // Stop source capture if the element gets disabled (do it inside the worker thread!)
    tmpConnection = connect(this, &SourceElement::stateChanged, sourceObject, [this] (bool active) {
        if (!active && sourceIface) {
            sourceIface->stopSource();
        }
    }, Qt::QueuedConnection);
    signalConnections.append(tmpConnection);

    // Ejection of source object from the element; this returns the
    // source object to the main thread, and either re-parents it
    // or schedules it for deletion. Must be connected with blocking
    // queued connection, so we can push the object from the worker
    // thread back to the main one...
    tmpConnection = connect(this, &SourceElement::ejectSource, sourceObject, [this] () {
        qInfo() << "Ejecting source" << sourceObject << "from source element in thread" << QThread::currentThread();

        // Stop the capture
        sourceIface->stopSource();

        // Push back to main thread
        sourceObject->moveToThread(QCoreApplication::instance()->thread());

        // Disconnect our custom signals
        for (QMetaObject::Connection &connection : signalConnections) {
            disconnect(connection);
        }

        // Re-parent or schedule for deletion
        if (sourceParent) {
            sourceObject->setParent(sourceParent);
        } else {
            sourceObject->deleteLater();
        }

        // Clear pointers
        signalConnections.clear();

        sourceObject = nullptr;
        sourceParent = nullptr;
        sourceIface = nullptr;

        // Clear cached image
        imageL = cv::Mat();
        imageR = cv::Mat();

        emit imagesChanged(cv::Mat(), cv::Mat());
    }, Qt::BlockingQueuedConnection); // Connection must block!
    signalConnections.append(tmpConnection);

    qInfo() << "Moving source to thread:" << thread;
}

QObject *SourceElement::getImagePairSource ()
{
    return sourceObject;
}


cv::Mat SourceElement::getLeftImage () const
{
    QReadLocker locker(&lock);
    return imageL.clone();
}

cv::Mat SourceElement::getRightImage () const
{
    QReadLocker locker(&lock);
    return imageL.clone();
}

void SourceElement::getImages (cv::Mat imageLeft, cv::Mat imageRight) const
{
    QReadLocker locker(&lock);
    imageL.copyTo(imageLeft);
    imageR.copyTo(imageRight);
}


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL