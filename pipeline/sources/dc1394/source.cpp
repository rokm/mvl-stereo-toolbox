/*
 * DC1394 Image Pair Source: source
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

using namespace SourceDC1394;


Source::Source (QObject *parent)
    : QAbstractListModel(parent), ImagePairSource()
{
    leftCamera = NULL;
    rightCamera = NULL;

    // FireWire bus
    fw = dc1394_new();
    if (!fw) {
        qWarning() << "Failed to create DC1394 object!";
    }

    // Enumerate cameras
    refreshCameraList();

    // Name
    shortName = "DC1394";
}

Source::~Source ()
{
    // Stop capture
    startStopCapture(false);

    // Release FireWire bus
    if (fw) {
        dc1394_free(fw);
    }
}


// *********************************************************************
// *                     ImagePairSource interface                     *
// *********************************************************************
QWidget *Source::createConfigWidget (QWidget *parent)
{
    return new SourceWidget(this, parent);
}


void Source::stopSource ()
{
    startStopCapture(false);
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
    if (!fw) {
        return;
    }
    dc1394error_t ret;   
    dc1394camera_list_t *camera_list;
    ret = dc1394_camera_enumerate(fw, &camera_list);
    if (ret) {
        qWarning() << "Failed to enumerate cameras; error code:" << ret;
        return;
    }

    if (camera_list->num) {
        beginInsertRows(QModelIndex(), 1, camera_list->num);
        for (unsigned int i = 0; i < camera_list->num; i++) {
            entries.append(camera_list->ids[i]);
        }
        active = QVector<bool>(camera_list->num, false);
        endInsertRows();
    }
    
    // Cleanup
    dc1394_camera_free_list(camera_list);
}


// *********************************************************************
// *                     Public camera management                      *
// *********************************************************************
int Source::getNumberOfCameras () const
{
    return entries.size();
}

const dc1394camera_id_t &Source::getCameraInfo (int c) const
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
    const dc1394camera_id_t &newId = entries[c];

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
    dc1394camera_t *raw_camera = dc1394_camera_new_unit(fw, newId.guid, newId.unit);
    if (!raw_camera) {
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
        dc1394camera_id_t id = camera->getId();

        // Disconnect
        disconnect(camera, SIGNAL(frameReady()), this, SLOT(synchronizeFrames()));

        // Delete camera object 
        delete camera;
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

void Source::setActive (const dc1394camera_id_t &id, bool value)
{
    for (int i = 0; i < entries.size(); i++) {
        const dc1394camera_id_t &storedId = entries[i];
        if (storedId.guid == id.guid && storedId.unit == id.unit) {
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
    const dc1394camera_id_t &entry = entries[index.row() - 1];
    switch (role) {
        case Qt::DisplayRole: {
            return QString("%1:%2").arg(entry.guid, 8, 16, QChar('0')).arg(entry.unit);
        }
        case Qt::UserRole: {
            // Index of device
            return index.row() - 1;
        }
    }

    return QVariant();
}
