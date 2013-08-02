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

using namespace SourceUnicap;


Source::Source (QObject *parent)
    : QAbstractListModel(parent), ImagePairSource()
{
    leftCamera = NULL;
    rightCamera = NULL;

    // Enumerate cameras
    refreshCameraList();
}

Source::~Source ()
{
    // Stop capture
    startStopCapture(false);
}


// *********************************************************************
// *                     ImagePairSource interface                     *
// *********************************************************************
QString Source::getShortName () const
{
    return "Unicap";
}

void Source::getImages (cv::Mat &left, cv::Mat &right)
{
    // Copy images under lock
    QReadLocker lock(&imagesLock);
    imageLeft.copyTo(left);
    imageRight.copyTo(right);
}

void Source::stopSource ()
{
    startStopCapture(false);
}

QWidget *Source::createConfigWidget (QWidget *parent)
{
    return new SourceWidget(this, parent);
}


// *********************************************************************
// *                        Camera enumeration                         *
// *********************************************************************
void Source::refreshCameraList ()
{
    // Release currently-set cameras
    releaseCamera(leftCamera);
    releaseCamera(rightCamera);

    // Clear old entries
    if (entries.size()) {
        beginRemoveRows(QModelIndex(), 1, entries.size()); // Remove all but first entry, which is "None"
        entries.clear();
        endRemoveRows();
    }
    
    // Enumerate devices
    int num_devices;
    unicap_status_t status = unicap_reenumerate_devices(&num_devices);
    if (status != STATUS_SUCCESS) {
        qWarning() << "Failed to re-enumerate devices!";
        return;
    }

    if (num_devices) {
        entries = QVector<unicap_device_t>(num_devices);
        active = QVector<bool>(num_devices, false);

        beginInsertRows(QModelIndex(), 1, num_devices);

        for (int i = 0; i < num_devices; i++) {
            status = unicap_enumerate_devices(NULL, &entries[i], i);
            if (!SUCCESS(status)) {
                qWarning() << "Failed to query device" << i;
            }
        }

        endInsertRows();
    }
}


// *********************************************************************
// *                     Public camera management                      *
// *********************************************************************
int Source::getNumberOfCameras () const
{
    return entries.size();
}

const unicap_device_t &Source::getCameraInfo (int c) const
{
    return entries[c];
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

Camera *Source::getLeftCamera ()
{
    return leftCamera;
}

Camera *Source::getRightCamera ()
{
    return rightCamera;
}


// *********************************************************************
// *                    Internal camera management                     *
// *********************************************************************
void Source::createCamera (Camera *& camera, int c)
{
    // If c is -1, release camera
    if (c == -1) {
        releaseCamera(camera);
        return;
    }

    // Get camera descriptor from our list
    unicap_device_t deviceInfo = entries[c];

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
    setActive(c, true);
}

void Source::releaseCamera (Camera *& camera)
{
    if (camera) {
        bool left = (camera == leftCamera);
        unicap_device_t device = camera->getDevice();

        // Disconnect
        disconnect(camera, SIGNAL(frameReady()), this, SLOT(synchronizeFrames()));

        // Delete camera object 
        camera->deleteLater();
        camera = NULL;

        // Mark camera as inactive in our list
        setActive(device, false);

        // Emit camera change
        if (left) {
            emit leftCameraChanged();
        } else {
            emit rightCameraChanged();
        }
    }
}

void Source::setActive (int c, bool value)
{
    active[c] = value;

    // Emit data changed
    dataChanged(index(c+1, 0), index(c+1, 0));
}

void Source::setActive (const unicap_device_t &dev, bool value)
{
    for (int i = 0; i < entries.size(); i++) {
        if (!strcmp(entries[i].identifier, dev.identifier)) {
            setActive(i, value);
            break;
        }
    }
}


// *********************************************************************
// *                              Capture                              *
// *********************************************************************
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


// *********************************************************************
// *                               Model                               *
// *********************************************************************
int Source::rowCount (const QModelIndex &) const
{
    return entries.size() + 1;
}

Qt::ItemFlags Source::flags (const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (index.row() == 0) {
        // Always selectable
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    } else {
        int i = index.row() - 1;
        // Selectable only if not enabled
        if (!active[i]) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        } else {
            return Qt::NoItemFlags;
        }
    }
}

QVariant Source::data (const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    // First element is always "None"
    if (index.row() == 0) {
        switch (role) {
            case Qt::DisplayRole: {
                return "None";
            }
            case Qt::ToolTipRole: {
                return "No device selected";
            }
            default: {
                return QVariant();
            }
        }

        return QVariant();
    }

    // Other valid devices
    const unicap_device_t &entry = entries[index.row() - 1];
    switch (role) {
        case Qt::DisplayRole: {
            return QString("%1").arg(entry.identifier);
        }
        case Qt::UserRole: {
            // Index of device
            return index.row() - 1;
        }
    }

    return QVariant();
}

