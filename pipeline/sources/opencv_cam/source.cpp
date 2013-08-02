/*
 * OpenCV Camera Image Pair Source: source
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

#include <opencv2/highgui/highgui.hpp>

using namespace SourceOpenCvCam;


// Supported interfaces on which we query. This separation is needed
// because the interface auto-selection code in OpenCV 2.4.5 looks
// fishy and does not seem to properly query all devices (from index 0)
// on all interfaces, but rather skips indices that were found on
// previously-queried interfaces.
static const struct {
    int id;
    const char *description;
} interfaces_list[] = {
    { CV_CAP_VFW, "Native" },// Native: V4L, V4L2 or VFW
    { CV_CAP_IEEE1394, "FireWire" }, // IEEE 1394 FireWire
    { CV_CAP_QT, "QuickTime" }, // QuickTime
    //{ CV_CAP_UNICAP, "UniCap" }, // Unicap
    { CV_CAP_DSHOW, "DirectShow" }, // DirectShow
    { CV_CAP_MSMF, "MSMF" }, // Microsoft Media Foundation
    { CV_CAP_PVAPI, "PVAPI" }, // Prosilica GigE SDK
    { CV_CAP_GIGANETIX, "GIGANETIX" }, // GigEVisionSSDK
};


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
    return "OpenCV CAM";
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

    // Enumerate cameras
    for (unsigned int i = 0; i < sizeof(interfaces_list)/sizeof(interfaces_list[0]); i++) {
        for (int j = 0; j < 99; j++) {
            try {
                cv::VideoCapture cap(interfaces_list[i].id + j);
                if (!cap.isOpened()) {
                    break;
                }
            } catch (...) {
                break;
            }

            ocv_camera_id_t entry;
            entry.interface_idx = i;
            entry.camera_idx = j;
            entries.append(entry);
        }
    }
    
    if (entries.size()) {
        beginInsertRows(QModelIndex(), 1, entries.size());
        active = QVector<bool>(entries.size(), false);
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

const ocv_camera_id_t& Source::getCameraInfo (int c) const
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

    // Get camera id from our list
    const ocv_camera_id_t &newId = entries[c];

    // Check if it is the same as the current right camera
    if (rightCamera && rightCamera->isSameCamera(newId)) {
        return;
    }

    // Check if it is the same as the current left camera
    if (leftCamera && leftCamera->isSameCamera(newId)) {
        return;
    }

    // Release current camera
    releaseCamera(camera);

    // Create new camera
    cv::VideoCapture *cap = NULL;
    int idx = interfaces_list[newId.interface_idx].id + newId.camera_idx;
    try {
        cap = new cv::VideoCapture(idx);
        if (!cap->isOpened()) {
            qWarning() << "Failed to open OpenCV camera capture for device" << idx << "!";
        }
    } catch (...) {
        qWarning() << "Failed to create OpenCV camera capture for device" << idx << "!";
    }

    if (!cap) {
        return;
    }
    
    camera = new Camera(cap, newId, this);

    // Connect
    connect(camera, SIGNAL(frameReady()), this, SLOT(synchronizeFrames()));
    // Mark camera as active in our list
    setActive(c, true);
}

void Source::releaseCamera (Camera *& camera)
{
    if (camera) {
        bool left = (camera == leftCamera);
        ocv_camera_id_t id = camera->getId();

        // Disconnect
        disconnect(camera, SIGNAL(frameReady()), this, SLOT(synchronizeFrames()));

        // Delete camera object 
        camera->deleteLater();
        camera = NULL;

        // Mark camera as inactive in our list
        setActive(id, false);

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

void Source::setActive (const ocv_camera_id_t &id, bool value)
{
    for (int i = 0; i < entries.size(); i++) {
        const ocv_camera_id_t &storedId = entries[i];
        if (storedId.interface_idx == id.interface_idx && storedId.camera_idx == id.camera_idx) {
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
    const ocv_camera_id_t &entry = entries[index.row() - 1];

    switch (role) {
        case Qt::DisplayRole: {
            return QString("%1: #%2").arg(interfaces_list[entry.interface_idx].description).arg(entry.camera_idx);
        }
        case Qt::UserRole: {
            // Device index
            return index.row() - 1;
        }
    }

    return QVariant();
}
