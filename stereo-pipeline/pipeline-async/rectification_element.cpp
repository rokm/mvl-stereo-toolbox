#include "rectification_element.h"

#include <stereo-pipeline/rectification.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace AsyncPipeline {


RectificationElement::RectificationElement (QObject *parent)
    : Element("Rectification", parent),
      rectification(new Rectification())
{
    // Update time and FPS statistics (local loop)
    connect(this, &RectificationElement::imagesChanged, this, &RectificationElement::incrementUpdateCount);

    // Move rectification object to worker thread
    rectification->moveToThread(thread);

    // Ejection of rectification object from the element; this pushes
    // the rectification object to the main thread, and schedules it
    // for deletion. Must be connected with blocking queued connection!
    connect(this, &RectificationElement::eject, rectification, [this] () {
        qInfo() << "Ejecting" << rectification << "from element in thread" << QThread::currentThread();

        // Push to main thread
        rectification->moveToThread(QCoreApplication::instance()->thread());

        // Schedule for deletion
        rectification->deleteLater();

        // Clear pointer
        rectification = nullptr;
    }, Qt::BlockingQueuedConnection); // Connection must block!

    // Main worker function - executed in rectification object's context,
    // and hence in the worker thread
    connect(this, &RectificationElement::imageRectificationRequest, rectification, [this] (const cv::Mat imageLeft, const cv::Mat imageRight) {
        QMutexLocker locker(&mutex);

        threadData.timer.start();
        try {
            rectification->rectifyImagePair(imageLeft, imageRight, threadData.imageL, threadData.imageR);
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

        threadData.imageL.copyTo(imageL);
        threadData.imageR.copyTo(imageR);
        lastOperationTime = threadData.processingTime;
        droppedCounter = 0; // Reset dropped-frame counter

        lock.unlock();

        // Signal change
        emit imagesChanged(imageLeft.clone(), imageRight.clone());
    }, Qt::QueuedConnection);

}

RectificationElement::~RectificationElement ()
{
    emit eject(); // Eject rectification object
}


Rectification *RectificationElement::getRectification ()
{
    return rectification;
}


cv::Mat RectificationElement::getLeftImage () const
{
    QReadLocker locker(&lock);
    return imageL.clone();
}

cv::Mat RectificationElement::getRightImage () const
{
    QReadLocker locker(&lock);
    return imageR.clone();
}

void RectificationElement::getImages (cv::Mat imageLeft, cv::Mat imageRight) const
{
    QReadLocker locker(&lock);
    imageL.copyTo(imageLeft);
    imageR.copyTo(imageRight);
}


cv::Mat RectificationElement::getReprojectionMatrix () const
{
    return rectification->getReprojectionMatrix();
}


void RectificationElement::rectifyImages (const cv::Mat &imageL, const cv::Mat &imageR)
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
        emit imageRectificationRequest(imageL.clone(), imageR.clone());
        mutex.unlock();
    } else {
        // Drop the frame
        lock.lockForWrite();
        droppedCounter++;
        lock.unlock();

        emit frameDropped();
    }
}


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL