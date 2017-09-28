/*
 * Stereo Pipeline: asynchronous pipeline: reprojection placeholder element
 * Copyright (C) 2017 Rok Mandeljc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
        //qInfo() << "Ejecting" << reprojection << "from element in thread" << QThread::currentThread();

        // Push to main thread
        reprojection->moveToThread(QCoreApplication::instance()->thread());

        // Schedule for deletion
        reprojection->deleteLater();

        // Clear pointer
        reprojection = Q_NULLPTR;
    }, Qt::BlockingQueuedConnection); // Connection must block!

    // Main worker function - executed in reprojection object's context,
    // and hence in the worker thread
    connect(this, &ReprojectionElement::reprojectionRequest, reprojection, [this] (const cv::Mat disparity) {
        QMutexLocker mutexLocker(&mutex);

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
        QWriteLocker locker(&lock);

        threadData.points.copyTo(points);
        lastOperationTime = threadData.processingTime;
        droppedCounter = 0; // Reset dropped-frame counter

        locker.unlock();

        // Signal change
        emit pointsChanged();
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

void ReprojectionElement::getPoints (cv::Mat &points) const
{
    QReadLocker locker(&lock);
    this->points.copyTo(points);
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
        dropFrame();
    }
}


} // AsyncPipeline
} // Pipeline
} // StereoToolbox
} // MVL
