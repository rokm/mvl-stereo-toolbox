/*
 * DC1394 Camera: camera
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
 
#include "CameraDC1394.h"
#include "CameraDC1394ConfigWidget.h"

#include <opencv2/imgproc/imgproc.hpp>

#define NUM_BUFFERS 32


CameraDC1394::CameraDC1394 (dc1394camera_t *c, QObject *parent)
    : QObject(parent), camera(c)
{
    // Print info
    dc1394_camera_print_info(camera, stdout);

    // Print features
    dc1394_feature_get_all(camera, &features);
    dc1394_feature_print_all(&features, stdout);

    // Create capture thread
    captureActive = false;
    captureThread = new QThread(this);
    connect(captureThread, SIGNAL(started()), this, SLOT(captureFunction()), Qt::DirectConnection);
    connect(this, SIGNAL(captureFinished()), captureThread, SLOT(quit()), Qt::DirectConnection);

    // Config widget
    configWidget = new CameraDC1394ConfigWidget(this);
}

CameraDC1394::~CameraDC1394 ()
{
    // Make sure capture is stopped
    stopCapture();

    // Unparent the config widget and destroy it
    configWidget->setParent(0);
    delete configWidget;
}

QWidget *CameraDC1394::getConfigWidget ()
{
    return configWidget;
}


// *********************************************************************
// *                       Camera identification                       *
// *********************************************************************
dc1394camera_id_t CameraDC1394::getId () const
{
    dc1394camera_id_t id;
    id.guid = camera->guid;
    id.unit = camera->unit;
    return id;
}

bool CameraDC1394::isSameCamera (const dc1394camera_id_t &id) const
{
    return dc1394_is_same_camera(getId(), id) == DC1394_TRUE;
}


QString CameraDC1394::getVendor () const
{
    return QString(camera->vendor);
}

QString CameraDC1394::getModel () const
{
    return QString(camera->model);
}


// *********************************************************************
// *                         Basic properties                          *
// *********************************************************************
// ISO speed
void CameraDC1394::setIsoSpeed (dc1394speed_t speed)
{
    dc1394error_t ret;

    ret = dc1394_video_set_iso_speed(camera, speed);
    if (ret) {
        emit error("Failed to set ISO speed!");
        return;
    }

    emit parameterChanged();
}

dc1394speed_t CameraDC1394::getIsoSpeed () const
{
    dc1394speed_t value;
    dc1394error_t ret;

    // Set mode
    ret = dc1394_video_get_iso_speed(camera, &value);
    if (ret) {
        qWarning() << "Failed to get ISO speed!";
    }
    
    return value;
}


// Video mode
QVector<dc1394video_mode_t> CameraDC1394::getSupportedModes ()
{
    dc1394video_modes_t raw_modes;
    dc1394error_t ret;

    QVector<dc1394video_mode_t> modes; 
    
    ret = dc1394_video_get_supported_modes(camera, &raw_modes);
    if (ret) {
        emit error("Failed to query supported modes!");
        return modes;
    }

    modes.resize(raw_modes.num);
    for (int i = 0; i < raw_modes.num; i++) {
        modes[i] = raw_modes.modes[i];
    }

    return modes;
}

void CameraDC1394::setMode (dc1394video_mode_t mode)
{
    dc1394error_t ret;

    // Set mode
    ret = dc1394_video_set_mode(camera, mode);
    if (ret) {
        emit error("Failed to set mode!");
        return;
    }

    emit parameterChanged();
}

dc1394video_mode_t CameraDC1394::getMode () const
{
    dc1394video_mode_t value;
    dc1394error_t ret;

    // Set mode
    ret = dc1394_video_get_mode(camera, &value);
    if (ret) {
        qWarning() << "Failed to get mode!";
    }
    
    return value;
}


// Framerate
QVector<dc1394framerate_t> CameraDC1394::getSupportedFramerates ()
{
    dc1394framerates_t raw_framerates;
    dc1394error_t ret;

    QVector<dc1394framerate_t> framerates; 
    
    ret = dc1394_video_get_supported_framerates(camera, getMode(), &raw_framerates);
    if (ret) {
        emit error("Failed to query supported framerates");
        return framerates;
    }

    framerates.resize(raw_framerates.num);
    for (int i = 0; i < raw_framerates.num; i++) {
        framerates[i] = raw_framerates.framerates[i];
    }

    return framerates;
}

void CameraDC1394::setFramerate (dc1394framerate_t fps)
{
    dc1394error_t ret;

    // Set framerate
    ret = dc1394_video_set_framerate(camera, fps);
    if (ret) {
        emit error("Failed to set framerate");
        return;
    }

    emit parameterChanged();
}

dc1394framerate_t CameraDC1394::getFramerate () const
{
    dc1394framerate_t value;
    dc1394error_t ret;

    // Set mode
    ret = dc1394_video_get_framerate(camera, &value);
    if (ret) {
        qWarning() << "Failed to get framerate!";
    }
    
    return value;
}


// *********************************************************************
// *                         Camera features                           *
// *********************************************************************
const dc1394featureset_t &CameraDC1394::getFeatureSet () const
{
    return features;
}


void CameraDC1394::setFeatureValue (dc1394feature_t feature, int newValue)
{
    dc1394error_t ret;

    ret = dc1394_feature_set_value(camera, feature, newValue);
    if (ret) {
        qDebug() << "Failed to set feature value!";
    }
}

int CameraDC1394::getFeatureValue (dc1394feature_t feature)
{
    quint32 value;
    dc1394error_t ret;
    
    ret = dc1394_feature_get_value(camera, feature, &value);
    if (ret) {
        qDebug() << "Failed to get feature value!";
    }

    return value;
}


QList<dc1394feature_mode_t> CameraDC1394::getFeatureModes (dc1394feature_t feature)
{
    dc1394error_t ret;
    dc1394feature_modes_t raw_modes;

    QList<dc1394feature_mode_t> modes;

    ret = dc1394_feature_get_modes(camera, feature, &raw_modes);
    if (ret) {
        qDebug() << "Failed to set feature value!";
        return modes;
    }

    for (int i = 0; i < raw_modes.num; i++) {
        modes.append(raw_modes.modes[i]);
    }

    return modes;
}

void CameraDC1394::setFeatureMode (dc1394feature_t feature, dc1394feature_mode_t mode)
{
    dc1394error_t ret;
    
    ret = dc1394_feature_set_mode(camera, feature, mode);
    if (ret) {
        qDebug() << "Failed to set feature mode!";
    }
}

dc1394feature_mode_t CameraDC1394::getFeatureMode (dc1394feature_t feature)
{
    dc1394feature_mode_t mode;
    dc1394error_t ret;
    
    ret = dc1394_feature_get_mode(camera, feature, &mode);
    if (ret) {
        qDebug() << "Failed to get feature mode!";
    }

    return mode;
}


// *********************************************************************
// *                         Capture start/stop                        *
// *********************************************************************
void CameraDC1394::captureFunction ()
{
    dc1394error_t ret;

    qDebug() << this << "Starting capture function...";

    // Setup capture
    ret = dc1394_capture_setup(camera, NUM_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT);
    if (ret) {
        qWarning() << "Could not setup camera! Make sure that the video mode and framerate are supported by the camera!";
        emit captureFinished();
        return;
    }

    // Start ISO transmission
    ret = dc1394_video_set_transmission(camera, DC1394_ON);
    if (ret) {
        qWarning() << "Could not start camera ISO transmission!";
        emit captureFinished();
        return;
    }

    captureActive = true;
    dc1394video_frame_t *frame;

    while (captureActive) {
        dequeueCaptureBuffer(frame, true);

        frameBufferLock.lockForWrite();
        convertToOpenCVImage(frame, frameBuffer);
        frameBufferLock.unlock();
        emit frameReady();

        enqueueCaptureBuffer(frame);
    }
    

    // Cleanup
    ret = dc1394_video_set_transmission(camera, DC1394_OFF);
    if (ret) {
        qWarning() << "Could not stop camera ISO transmission!";
    }
    
    ret = dc1394_capture_stop(camera);
    if (ret) {
        qWarning() << "Could not stop camera capture!";

    }

    qDebug() << this << "Capture function ended!";
    emit captureFinished();
}

void CameraDC1394::startCapture ()
{
    if (!captureThread->isRunning()) {
        // Start capture thread
        captureThread->start();
    }
}

void CameraDC1394::stopCapture ()
{
    if (captureThread->isRunning()) {
        captureActive = false;

        // Make sure capture thread finishes
        captureThread->wait();
    }
}

bool CameraDC1394::getCaptureState () const
{
    return captureActive;
}


// *********************************************************************
// *                    Advanced grabbing interface                    *
// *********************************************************************
void CameraDC1394::dequeueCaptureBuffer (dc1394video_frame_t *&frame, bool drainQueue)
{
    dc1394error_t ret;
    
    // Dequeue
    ret = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
    if (ret) {
        qWarning() << "Could not dequeue frame!";
        return;
    }

    if (drainQueue) {
        // Drain until we're no frames behind
        while (frame->frames_behind) {
            qDebug() << "Draining capture queue due to being" << frame->frames_behind << "frames!";
            ret = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
            if (ret) {
                qWarning() << "Could not dequeue frame!";
                return;
            }
        }
    }
}

void CameraDC1394::enqueueCaptureBuffer (dc1394video_frame_t *frame)
{
    dc1394error_t ret;

    // Enqueue
    ret = dc1394_capture_enqueue (camera, frame);
    if (ret) {
        qWarning() << "Could not enque buffer!";
        return;
    }
}

void CameraDC1394::convertToOpenCVImage (dc1394video_frame_t *frame, cv::Mat &image) const
{
    // Determine if image is monochrome or color
    dc1394bool_t isColor = DC1394_FALSE;
    dc1394_is_color(frame->color_coding, &isColor);

    if (isColor == DC1394_TRUE) {
        // Convert into RGB8...
        cv::Mat tmpImg(frame->size[1], frame->size[0], CV_8UC3);
        dc1394_convert_to_RGB8(frame->image, tmpImg.ptr<uint8_t>(), frame->size[0], frame->size[1], frame->yuv_byte_order, frame->color_coding, frame->data_depth);

        // ... and then RGB to BGR, which is what OpenCV needs
        cv::cvtColor(tmpImg, image, CV_RGB2BGR);
    } else {
        // Convert into Mono8
        image.create(frame->size[1], frame->size[0], CV_8UC1); // (Re)allocate, if necessary
        dc1394_convert_to_MONO8(frame->image, image.ptr<uint8_t>(), frame->size[0], frame->size[1], frame->yuv_byte_order, frame->color_coding, frame->data_depth);
    }
}

// *********************************************************************
// *                           Frame access                            *
// *********************************************************************
void CameraDC1394::copyFrame (cv::Mat &frame)
{
    // Copy under lock
    QReadLocker lock(&frameBufferLock);
    frameBuffer.copyTo(frame);
}
