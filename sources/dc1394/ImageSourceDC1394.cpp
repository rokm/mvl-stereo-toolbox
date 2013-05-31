/*
 * DC1394 Image Source: source
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include "ImageSourceDC1394.h"
#include "ImageSourceDC1394ConfigWidget.h"

#include "CameraDC1394.h"
#include "CameraListModel.h"


ImageSourceDC1394::ImageSourceDC1394 (QObject *parent)
    : ImageSource(parent)
{
    fw = NULL;
    leftCamera = NULL;
    rightCamera = NULL;

    // Camera list
    cameraListModel = new CameraListModel(this);

    // Perform initial scan
    scanBus();

    // Name
    shortName = "DC1394";
}

ImageSourceDC1394::~ImageSourceDC1394 ()
{
    // Stop capture
    startStopCapture(false);

    // Release FireWire bus
    if (fw) {
        dc1394_free(fw);
    }
}


QWidget *ImageSourceDC1394::createConfigWidget (QWidget *parent)
{
    return new ImageSourceDC1394ConfigWidget(this, parent);
}



CameraListModel *ImageSourceDC1394::getCameraListModel ()
{
    return cameraListModel;
}


void ImageSourceDC1394::scanBus ()
{
    dc1394error_t ret;

    // Release old cameras
    releaseCamera(leftCamera);
    releaseCamera(rightCamera);

    qDebug() << "Scanning bus for DC1394 devices...";
    
    // Create DC1394 object - if not already created
    if (!fw) {
        fw = dc1394_new();
    }
    if (!fw) {
        qWarning() << "Failed to create DC1394 object!";
        return;
    }

    // Enumerate cameras    
    dc1394camera_list_t *camera_list;

    ret = dc1394_camera_enumerate(fw, &camera_list);
    if (ret) {
        qWarning() << "Failed to enumerate cameras; error code:" << ret;
        return;
    }
    cameraListModel->setDeviceList(camera_list);
    dc1394_camera_free_list(camera_list);
    
}


void ImageSourceDC1394::setLeftCamera (int c)
{
    // Create camera
    createCamera(leftCamera, c);
    emit leftCameraChanged();
}

void ImageSourceDC1394::setRightCamera (int c)
{
    // Create camera
    createCamera(rightCamera, c);
    emit rightCameraChanged();
}

void ImageSourceDC1394::createCamera (CameraDC1394 *& camera, int c)
{
    // If c is -1, release camera
    if (c == -1) {
        releaseCamera(camera);
        return;
    }

    // Get camera id from our list
    const dc1394camera_id_t &newId = cameraListModel->getDeviceId(c);

    // Check if it is the same as the current right camera
    if (rightCamera && rightCamera->isSameCamera(newId)) {
        qDebug() << "Same as current right!";
        return;
    }

    // Check if it is the same as the current left camera
    if (leftCamera && leftCamera->isSameCamera(newId)) {
        qDebug() << "Same as current left!";
        return;
    }

    // Release current camera
    releaseCamera(camera);

    // Create new camera
    dc1394camera_t *raw_camera = dc1394_camera_new_unit(fw, newId.guid, newId.unit);
    if (!raw_camera) {
        qWarning() << "Failed to create camera object!";
        return;
    }
    camera = new CameraDC1394(raw_camera, this);

    // Connect
    connect(camera, SIGNAL(frameReady()), this, SLOT(frameAggregator()));

    // Mark camera as active in our list
    cameraListModel->setActive(c, true);
}

void ImageSourceDC1394::releaseCamera (CameraDC1394 *& camera)
{
    if (camera) {
        dc1394camera_id_t id = camera->getId();

        // Disconnect
        disconnect(camera, SIGNAL(frameReady()), this, SLOT(frameAggregator()));

        // Delete camera object 
        delete camera;
        camera = NULL;

        // Mark camera as inactive in our list
        cameraListModel->setActive(id, false);
    }
}


CameraDC1394 *ImageSourceDC1394::getLeftCamera ()
{
    return leftCamera;
}

CameraDC1394 *ImageSourceDC1394::getRightCamera ()
{
    return rightCamera;
}


void ImageSourceDC1394::startStopCapture (bool start)
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

void ImageSourceDC1394::frameAggregator ()
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
