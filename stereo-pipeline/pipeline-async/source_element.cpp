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
      sourceIface(nullptr),
      framerateLimit(0.0)
{
    // Update time and FPS statistics (local loop)
    connect(this, &SourceElement::imagesChanged, this, &SourceElement::incrementUpdateCount);
}

SourceElement::~SourceElement ()
{
    emit eject(); // Eject source
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
    emit eject();

    // Insert new source
    sourceObject = newSource;
    sourceIface = newSourceIface;

    sourceParent = sourceObject->parent(); // Store parent
    sourceObject->setParent(nullptr);

    sourceObject->moveToThread(thread);

    tmpConnection = connect(sourceObject, SIGNAL(imagesChanged()), this, SLOT(handleImagesChange()), Qt::QueuedConnection); // Must use old syntax because signal is defined in interface!
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
    tmpConnection = connect(this, &SourceElement::eject, sourceObject, [this] () {
        qInfo() << "Ejecting" << sourceObject << "from element; worker thread" << QThread::currentThread();

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
        QWriteLocker locker(&lock);

        imageL = cv::Mat();
        imageR = cv::Mat();

        locker.unlock();

        emit imagesChanged();
    }, Qt::BlockingQueuedConnection); // Connection must block!
    signalConnections.append(tmpConnection);

    qInfo() << "Moving source to thread:" << thread;

    // Update images from the new source
    QWriteLocker locker(&lock);
    sourceIface->getImages(imageL, imageR);
    locker.unlock();

    emit imagesChanged();
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
    return imageR.clone();
}

void SourceElement::getImages (cv::Mat &imageLeft, cv::Mat &imageRight) const
{
    QReadLocker locker(&lock);
    imageL.copyTo(imageLeft);
    imageR.copyTo(imageRight);
}

void SourceElement::handleImagesChange ()
{
    // Enforce framerate limit, if set
    if (framerateLimit != 0.0) {
        if (timeLastUpdate.elapsed() < 1000/framerateLimit) {
            // Drop frame by not updating the images
            dropFrame();
            return;
        } else {
            // We will update below; so reset the timer
            timeLastUpdate.restart();
            droppedCounter = 0; // Reset dropped-frame counter
        }
    }

    // Update images
    QWriteLocker locker(&lock);
    sourceIface->getImages(imageL, imageR);
    locker.unlock();

    emit imagesChanged();
}


void SourceElement::setFramerateLimit (double limit)
{
    if (framerateLimit != limit) {
        framerateLimit = limit;
        timeLastUpdate.restart(); // Restart the limiter timer
        emit framerateLimitChanged(framerateLimit);
    }
}

double SourceElement::getFramerateLimit () const
{
    return framerateLimit;
}


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL
