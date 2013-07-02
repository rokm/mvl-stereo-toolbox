/*
 * Unicap Image Pair Source: source
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

#include "source.h"
#include "source_widget.h"
#include "camera.h"
#include "camera_list.h"

using namespace SourceUnicap;


Source::Source (QObject *parent)
    : ImagePairSource(parent)
{
    leftCamera = NULL;
    rightCamera = NULL;

    // Camera list
    cameraList = new CameraList(this);

    // Perform initial scan
    scanForDevices();

    // Name
    shortName = "Unicap";
}

Source::~Source ()
{
    // Stop capture
    startStopCapture(false);
}


QWidget *Source::createConfigWidget (QWidget *parent)
{
    return new SourceWidget(this, parent);
}


void Source::stopSource ()
{
    startStopCapture(false);
}


CameraList *Source::getCameraList ()
{
    return cameraList;
}


void Source::scanForDevices ()
{
    // Release old cameras
    releaseCamera(leftCamera);
    releaseCamera(rightCamera);

    // Enumerate devices
    int num_devices;
    unicap_status_t status = unicap_reenumerate_devices(&num_devices);
    if (status != STATUS_SUCCESS) {
        qWarning() << "Failed to re-enumerate devices!";
        return;
    }

    QVector<unicap_device_t> devices = QVector<unicap_device_t>(num_devices);

    for (int i = 0; i < num_devices; i++) {
        status = unicap_enumerate_devices(NULL, &devices[i], i);
        if (!SUCCESS(status)) {
            qWarning() << "Failed to query device" << i;
        }
    }
    
    cameraList->setDeviceList(devices);    
}

void Source::setLeftCamera (int c)
{
    // Create camera
    createCamera(leftCamera, c);
    emit leftCameraChanged();
}

void Source::setRightCamera (int c)
{
    // Create camera
    createCamera(rightCamera, c);
    emit rightCameraChanged();
}

void Source::createCamera (Camera *& camera, int c)
{
    // If c is -1, release camera
    if (c == -1) {
        releaseCamera(camera);
        return;
    }

    // Get camera descriptor from our list
    unicap_device_t deviceInfo = cameraList->getDeviceInfo(c);

    // Check if it is the same as the current right camera
    if (rightCamera && rightCamera->isSameCamera(deviceInfo)) {
        qDebug() << "Same as current right!";
        return;
    }

    // Check if it is the same as the current left camera
    if (leftCamera && leftCamera->isSameCamera(deviceInfo)) {
        qDebug() << "Same as current left!";
        return;
    }

    // Release current camera
    releaseCamera(camera);

    // Create new camera
    unicap_handle_t raw_camera;
    unicap_status_t status = unicap_open(&raw_camera, &deviceInfo);
    if (status != STATUS_SUCCESS) {
        qWarning() << "Failed to create camera object!";
        return;
    }
    camera = new Camera(raw_camera, this);

    // Connect
    connect(camera, SIGNAL(frameReady()), this, SLOT(synchronizeFrames()));

    // Mark camera as active in our list
    cameraList->setActive(c, true);
}

void Source::releaseCamera (Camera *& camera)
{
    if (camera) {
        bool left = (camera == leftCamera);
        unicap_device_t device = camera->getDevice();

        // Disconnect
        disconnect(camera, SIGNAL(frameReady()), this, SLOT(synchronizeFrames()));

        // Delete camera object 
        delete camera;
        camera = NULL;

        // Mark camera as inactive in our list
        cameraList->setActive(device, false);

        // Emit camera change
        if (left) {
            emit leftCameraChanged();
        } else {
            emit rightCameraChanged();
        }
    }
}


Camera *Source::getLeftCamera ()
{
    return leftCamera;
}

Camera *Source::getRightCamera ()
{
    return rightCamera;
}


void Source::startStopCapture (bool start)
{
    if (start) {
        leftFrameReady = rightFrameReady = false;
        if (leftCamera) leftCamera->startCapture();
        if (rightCamera) rightCamera->startCapture();
    } else {
        if (leftCamera) leftCamera->stopCapture();
        if (rightCamera) rightCamera->stopCapture();
    }
}


// *********************************************************************
// *                        Frame synchronizer                         *
// *********************************************************************
void Source::synchronizeFrames ()
{
    if (QObject::sender() == leftCamera) {
        leftFrameReady = true;
    } else if (QObject::sender() == rightCamera) {
        rightFrameReady = true;
    }
    
    bool requireLeft = (leftCamera && leftCamera->getCaptureState());
    bool requireRight = (rightCamera && rightCamera->getCaptureState());

    if ((!requireLeft || leftFrameReady) && (!requireRight || rightFrameReady)) {
        if (requireLeft) {
            leftCamera->copyFrame(imageLeft);
        } else {
            imageLeft = cv::Mat();
        }
        if (requireRight) {
            rightCamera->copyFrame(imageRight);
        } else {
            imageRight = cv::Mat();
        }
        leftFrameReady = false;
        rightFrameReady = false;

        emit imagesChanged();
    }
}
