#include "reprojection_element.h"

#include <stereo-pipeline/reprojection.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace AsyncPipeline {


ReprojectionElement::ReprojectionElement (QObject *parent)
    : Element("Reprojection", parent),
      reprojection(new Reprojection())
{
    // Update time and FPS statistics (local loop)
    connect(this, &ReprojectionElement::pointsChanged, this, &ReprojectionElement::incrementUpdateCount);

    // Move reprojection object to worker thread
    reprojection->moveToThread(thread);

    // Ejection of reprojection object from the element; this pushes
    // the reprojection object to the main thread, and schedules it
    // for deletion. Must be connected with blocking queued connection!
    connect(this, &ReprojectionElement::eject, reprojection, [this] () {
        qInfo() << "Ejecting" << reprojection << "from element in thread" << QThread::currentThread();

        // Push to main thread
        reprojection->moveToThread(QCoreApplication::instance()->thread());

        // Schedule for deletion
        reprojection->deleteLater();

        // Clear pointer
        reprojection = nullptr;
    }, Qt::BlockingQueuedConnection); // Connection must block!

    // Main worker function - executed in reprojection object's context,
    // and hence in the worker thread
    connect(this, &ReprojectionElement::reprojectionRequest, reprojection, [this] (const cv::Mat disparity) {
        QMutexLocker locker(&mutex);

        threadData.timer.start();
        try {
            reprojection->reprojectDisparity(disparity, threadData.points);
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

        threadData.points.copyTo(points);
        lastOperationTime = threadData.processingTime;
        droppedCounter = 0; // Reset dropped-frame counter

        lock.unlock();

        // Signal change
        emit pointsChanged(points.clone());
    }, Qt::QueuedConnection);

}

ReprojectionElement::~ReprojectionElement ()
{
    emit eject(); // Eject Reprojection object
}


Reprojection *ReprojectionElement::getReprojection ()
{
    return reprojection;
}


cv::Mat ReprojectionElement::getPoints () const
{
    QReadLocker locker(&lock);
    return points.clone();
}


void ReprojectionElement::reprojectDisparity (const cv::Mat &disparity, int numDisparityLevels)
{
    // No-op if inactive
    if (!getState()) {
        return;
    }

    // No-op if disparity is empty
    if (disparity.empty()) {
        return;
    }

    // Try acquiring mutex to see if worker thread is busy processing
    if (mutex.tryLock()) {
        // Submit the task
        emit reprojectionRequest(disparity.clone());
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
