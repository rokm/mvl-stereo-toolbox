/*
 * OpenCV Camera: camera
 * Copyright (C) 2013 Rok Mandeljc
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
 
#include "camera.h"
#include "camera_widget.h"

#include <opencv2/imgproc.hpp>

#define NUM_BUFFERS 32

using namespace SourceOpenCvCam;


Camera::Camera (cv::VideoCapture *cap, ocv_camera_id_t cid, QObject *parent)
    : QObject(parent), capture(cap), id(cid)
{
}

Camera::~Camera ()
{
    // Make sure capture is stopped
    stopCapture();

    // Free camera
    delete capture;
}

QWidget *Camera::createConfigWidget (QWidget *parent)
{
    return new CameraWidget(this, parent);
}


// *********************************************************************
// *                       Camera identification                       *
// *********************************************************************
const ocv_camera_id_t &Camera::getId () const
{
    return id;
}

bool Camera::isSameCamera (const ocv_camera_id_t &otherId) const
{
    return id.interface_idx == otherId.interface_idx && id.camera_idx == otherId.camera_idx;
}


// *********************************************************************
// *                            Properties                             *
// *********************************************************************
double Camera::getProperty (int prop)
{
    return capture->get(prop);
}

void Camera::setProperty (int prop, double newValue)
{
    if (!capture->set(prop, newValue)) {
        emit error(QString("Failed to set property %1!").arg(prop));
    }
    
    emit propertyChanged();
}


// *********************************************************************
// *                              Capture                              *
// *********************************************************************
void Camera::startCapture ()
{
    if (!captureWatcher.isRunning()) {
        // Start capture function
        QFuture<void> future = QtConcurrent::run(this, &Camera::captureFunction);
        captureWatcher.setFuture(future);
    } else {
        //qWarning() << this << "Capture already running!";
    }
 }
 
void Camera::stopCapture ()
{
    if (captureWatcher.isRunning()) {
        captureActive = false;

        // Make sure capture thread finishes
        captureWatcher.waitForFinished();
    } else {
        //qWarning() << this << "Capture not running!";
    }
}

bool Camera::getCaptureState () const
{
    return captureWatcher.isRunning();
}

void Camera::captureFunction ()
{
    bool captureSucceeded;
    captureActive = true;

    while (captureActive) {
        captureSucceeded = capture->grab();

        frameBufferLock.lockForWrite();

        if (captureSucceeded) {
            captureSucceeded = capture->retrieve(frameBuffer);
        }

        if (!captureSucceeded) {
            frameBuffer = cv::Mat();
            captureActive = false;
        }

        frameBufferLock.unlock();

        emit frameReady();        
    }

    emit captureFinished();
}


// *********************************************************************
// *                           Frame access                            *
// *********************************************************************
void Camera::copyFrame (cv::Mat &frame)
{
    // Copy under lock
    QReadLocker lock(&frameBufferLock);
    frameBuffer.copyTo(frame);
}


