#include "method_element.h"

#include <stereo-pipeline/stereo_method.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace AsyncPipeline {


MethodElement::MethodElement (QObject *parent)
    : Element("StereoMethod", parent),
      methodObject(nullptr),
      methodParent(nullptr),
      methodIface(nullptr)
{
    // Update time and FPS statistics (local loop)
    connect(this, &MethodElement::disparityChanged, this, &MethodElement::incrementUpdateCount);
}

MethodElement::~MethodElement ()
{
    emit eject(); // Eject method
}


void MethodElement::setStereoMethod (QObject *newMethod)
{
    QMetaObject::Connection tmpConnection;

    // Try to get the interface
    StereoMethod *newMethodIface = qobject_cast<StereoMethod *>(newMethod);
    if (!newMethodIface) {
        qWarning() << "Passed object does not implement StereoMethod interface!";
        return;
    }

    // Eject old method
    emit eject();

    // Insert new method
    methodObject = newMethod;
    methodIface = newMethodIface;

    methodParent = methodObject->parent(); // Store parent
    methodObject->setParent(nullptr);

    methodObject->moveToThread(thread);

    // Ejection of source object from the element; this returns the
    // source object to the main thread, and either re-parents it
    // or schedules it for deletion. Must be connected with blocking
    // queued connection, so we can push the object from the worker
    // thread back to the main one...
    tmpConnection = connect(this, &MethodElement::eject, methodObject, [this] () {
        qInfo() << "Ejecting" << methodObject << "from element; worker thread" << QThread::currentThread();

        // Push back to main thread
        methodObject->moveToThread(QCoreApplication::instance()->thread());

        // Disconnect our custom signals
        for (QMetaObject::Connection &connection : signalConnections) {
            disconnect(connection);
        }

        // Re-parent or schedule for deletion
        if (methodParent) {
            methodObject->setParent(methodParent);
        } else {
            methodObject->deleteLater();
        }

        // Clear pointers
        signalConnections.clear();

        methodObject = nullptr;
        methodParent = nullptr;
        methodIface = nullptr;

        // Clear cached image
        disparity = cv::Mat();

        emit disparityChanged(cv::Mat(), 0);
    }, Qt::BlockingQueuedConnection); // Connection must block!
    signalConnections.append(tmpConnection);


    // Main worker function - executed in method object's context, and
    // hence in the worker thread
    tmpConnection = connect(this, &MethodElement::disparityComputationRequest, methodObject, [this] (const cv::Mat imageL, const cv::Mat imageR) {
        QMutexLocker locker(&mutex);

        threadData.timer.start();
        try {
            methodIface->computeDisparity(imageL, imageR, threadData.disparity, threadData.numDisparityLevels);
        } catch (const QString &message) {
            emit error(message);
            return;
        } catch (const std::exception &exception) {
            emit error("Exception: " + QString::fromStdString(exception.what()));
            return;
        } catch (...) {
            emit error("Unknown exception");
            return;
        }

        threadData.processingTime = threadData.timer.elapsed();

        // Store results
        lock.lockForWrite();

        threadData.disparity.copyTo(disparity);
        numDisparityLevels = threadData.numDisparityLevels;
        lastOperationTime = threadData.processingTime;
        droppedCounter = 0; // Reset dropped-frame counter

        lock.unlock();

        // Signal change
        emit disparityChanged(disparity.clone(), numDisparityLevels);
    }, Qt::QueuedConnection);
}

QObject *MethodElement::getStereoMethod ()
{
    return methodObject;
}


void MethodElement::loadParameters (const QString &filename)
{
    if (!methodIface) {
        throw QString("Method not set!");
    }

    QMutexLocker locker(&mutex);
    methodIface->loadParameters(filename);
}

void MethodElement::saveParameters (const QString &filename) const
{
    if (!methodIface) {
        throw QString("Method not set!");
    }

    QMutexLocker locker(&mutex);
    methodIface->saveParameters(filename);
}

void MethodElement::computeDisparity (const cv::Mat &imageL, const cv::Mat &imageR)
{
    // No-op if inactive
    if (!getState()) {
        return;
    }

    // No-op if one of images is empty
    if (imageL.empty() || imageR.empty()) {
        return;
    }

    // Try acquiring mutex to see if worker thread is busy processing
    if (mutex.tryLock()) {
        // Submit the task
        emit disparityComputationRequest(imageL.clone(), imageR.clone());
        mutex.unlock();
    } else {
        // Drop the frame
        lock.lockForWrite();
        droppedCounter++;
        lock.unlock();

        emit frameDropped();
    }
}

cv::Mat MethodElement::getDisparity () const
{
    QReadLocker locker(&lock);
    return disparity.clone();
}

void MethodElement::getDisparity (cv::Mat &disparity, int &numDisparityLevels) const
{
    QReadLocker locker(&lock);
    this->disparity.copyTo(disparity);
    numDisparityLevels = this->numDisparityLevels;
}


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL
