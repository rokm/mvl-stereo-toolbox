/*
 * Stereo Pipeline: asynchronous pipeline: visualization placeholder element
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

#include "visualization_element.h"

#include <stereo-pipeline/disparity_visualization.h>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace AsyncPipeline {


VisualizationElement::VisualizationElement (QObject *parent)
    : Element("Visualization", parent),
      visualization(new DisparityVisualization())
{
    // Propagate the method change signal
    connect(visualization, &DisparityVisualization::visualizationMethodChanged, this, &VisualizationElement::visualizationMethodChanged);

    // Update time and FPS statistics (local loop)
    connect(this, &VisualizationElement::imageChanged, this, &VisualizationElement::incrementUpdateCount);

    // Move visualization object to worker thread
    visualization->moveToThread(thread);

    // Ejection of visualization object from the element; this pushes
    // the visualization object to the main thread, and schedules it
    // for deletion. Must be connected with blocking queued connection!
    connect(this, &VisualizationElement::eject, visualization, [this] () {
        //qInfo() << "Ejecting" << visualization << "from element in thread" << QThread::currentThread();

        // Push to main thread
        visualization->moveToThread(QCoreApplication::instance()->thread());

        // Schedule for deletion
        visualization->deleteLater();

        // Clear pointer
        visualization = nullptr;
    }, Qt::BlockingQueuedConnection); // Connection must block!

    // Main worker function - executed in visualization object's context,
    // and hence in the worker thread
    connect(this, &VisualizationElement::disparityVisualizationRequest, visualization, [this] (const cv::Mat disparity, int numDisparityLevels) {
        QMutexLocker mutexLocker(&mutex);

        threadData.timer.start();
        try {
            visualization->visualizeDisparity(disparity, numDisparityLevels, threadData.image);
        } catch (const std::exception &e) {
            emit error(QString::fromStdString(e.what()));
            return;
        } catch (...) {
            emit error("Unhandled exception type!");
            return;
        }

        threadData.processingTime = threadData.timer.elapsed();

        // Store results
        QWriteLocker locker(&lock);

        threadData.image.copyTo(image);
        lastOperationTime = threadData.processingTime;
        droppedCounter = 0; // Reset dropped-frame counter

        locker.unlock();

        // Signal change
        emit imageChanged();
    }, Qt::QueuedConnection);
}

VisualizationElement::~VisualizationElement ()
{
    emit eject(); // Eject rectification object
}


DisparityVisualization *VisualizationElement::getVisualization ()
{
    return visualization;
}


cv::Mat VisualizationElement::getImage () const
{
    QReadLocker locker(&lock);
    return image.clone();
}

void VisualizationElement::getImage (cv::Mat &image) const
{
    QReadLocker locker(&lock);
    this->image.copyTo(image);
}


void VisualizationElement::visualizeDisparity (const cv::Mat &disparity, int numDisparityLevels)
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
        emit disparityVisualizationRequest(disparity.clone(), numDisparityLevels);
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
