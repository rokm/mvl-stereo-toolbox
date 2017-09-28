/*
 * Unicap Source: camera
 * Copyright (C) 2013-2017 Rok Mandeljc
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


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceUnicap {


static void callback_new_frame (unicap_event_t event, unicap_handle_t hande, unicap_data_buffer_t *buffer, void *usr_data);

#define FOURCC(a,b,c,d) (unsigned int)((((unsigned int)a))+(((unsigned int)b)<<8)+(((unsigned int)c)<<16)+(((unsigned int)d)<<24))

Camera::Camera (unicap_handle_t handle, QObject *parent)
    : QObject(parent),
      handle(handle)
{
    int num_formats;
    int num_properties;

    // Store device info
    unicap_get_device(handle, &device);

    // Enumerate formats
    int defaultFormat = 0;
    unicap_reenumerate_formats(handle, &num_formats);
    formats.resize(num_formats);
    for (int i = 0; i < num_formats; i++) {
        unicap_enumerate_formats(handle, NULL, &formats[i], i);

        if (formats[i].fourcc == FOURCC('Y', '8', '0', '0') ||
            formats[i].fourcc == FOURCC('B', 'G', 'R', '3')) {
            defaultFormat = i;
        }
    }

    // Set default format; try finding BGR3 or Y (Mono), if neither
    // exists, set the first format;

    // Set first available format
    setFormat(formats[defaultFormat]);

    // Get all properties
    unicap_reenumerate_properties(handle, &num_properties);
    properties.resize(num_properties);
    for (int i = 0; i < num_properties; i++) {
        unicap_enumerate_properties(handle, NULL, &properties[i], i);
    }

    // Use system buffers for capture; in this mode, unicap internally
    // manages a capture thread
    captureActive = false;
    unicap_register_callback(handle, UNICAP_EVENT_NEW_FRAME, (unicap_callback_t)callback_new_frame, (void *)this);
}

Camera::~Camera ()
{
    // Make sure capture is stopped
    stopCapture();

    // Free camera
    unicap_close(handle);
}

QWidget *Camera::createConfigWidget (QWidget *parent)
{
    return new CameraWidget(this, parent);
}


// *********************************************************************
// *                       Camera identification                       *
// *********************************************************************
unicap_device_t Camera::getDevice () const
{
    return device;
}

bool Camera::isSameCamera (const unicap_device_t &otherDevice) const
{
    return !strcmp(device.identifier, otherDevice.identifier);
}


// *********************************************************************
// *                       Hardware information                        *
// *********************************************************************
QString Camera::getDeviceVendor () const
{
    return QString(device.vendor_name);
}

QString Camera::getDeviceModel () const
{
    return QString(device.model_name);
}

QString Camera::getDevicePluginName () const
{
    return QString(device.cpi_layer);
}

QString Camera::getDeviceFileName () const
{
    return QString(device.device);
}


// *********************************************************************
// *                              Format                               *
// *********************************************************************
const QVector<unicap_format_t> &Camera::getSupportedFormats () const
{
    return formats;
}

void Camera::setFormat (const unicap_format_t &newFormat)
{
    unicap_status_t status;
    unicap_format_t tmp_format = newFormat;

    // Use maximum size for initialization
    tmp_format.size = tmp_format.max_size;

    // Set format
    status = unicap_set_format(handle, &tmp_format);
    if (status != STATUS_SUCCESS) {
        qWarning() << "Failed to set new format:" << status;
    }

    // Update format
    updateFormat();
}

void Camera::updateFormat ()
{
    // Read format from device...
    unicap_get_format(handle, &format);
    // ... and notify listeners of the change
    emit formatChanged();
}

const unicap_format_t &Camera::getFormat () const
{
    return format;
}


void Camera::setSize (unicap_rect_t &size)
{
    unicap_status_t status;

    // Copy format
    unicap_format_t newFormat = format;
    newFormat.size = size;

    // Set format
    status = unicap_set_format(handle, &newFormat);
    if (status != STATUS_SUCCESS) {
        qWarning() << "Failed to set new format:" << status;
    }

    // Read format from device...
    unicap_get_format(handle, &format);
    // ... and notify listeners of the change
    emit sizeChanged();
}


// *********************************************************************
// *                            Properties                             *
// *********************************************************************
const QVector<unicap_property_t> &Camera::getSupportedProperties () const
{
    return properties;
}

void Camera::updateProperty (unicap_property_t &property)
{
    unicap_status_t status;

    status = unicap_get_property(handle, &property);
    if (status != STATUS_SUCCESS) {
        qWarning() << "Failed to retrieve property:" << property.identifier << "; status:" << status;
    }
}

void Camera::setPropertyValue (const QString &name, double value)
{
    unicap_status_t status;

    status = unicap_set_property_value(handle, name.toLocal8Bit().data(), value);
    if (status != STATUS_SUCCESS) {
        qWarning() << "Failed to set value for" << name << "to" << value;
    }

    emit propertyChanged();
}

void Camera::setPropertyValue (const QString &name, const QString &value)
{
    unicap_status_t status;
    unicap_property_t property;

    // Find this property in our list
    bool found = false;
    for (unicap_property_t &p : properties) {
        if (name.compare(p.identifier)) {
            property = p;
            found = true;
            break;
        }
    }
    if (!found) {
        qWarning() << "Property" << name << "not found!";
        return;
    }

    // Query
    status = unicap_get_property(handle, &property);
    if (status != STATUS_SUCCESS) {
        qWarning() << "Failed to query property" << name << "!";
    }

    // Set menu value
    QByteArray value_str = value.toLocal8Bit();
    strncpy(property.menu_item, value_str, sizeof(property.menu_item));

    status = unicap_set_property(handle, &property);
    if (status != STATUS_SUCCESS) {
        qWarning() << "Failed to set property" << name << "to" << value;
    }

    emit propertyChanged();
}


void Camera::setPropertyMode (const QString &name, PropertyMode mode)
{
    unicap_status_t status = STATUS_SUCCESS;

    switch (mode) {
        case PropertyModeManual: {
            status = unicap_set_property_manual(handle, name.toLocal8Bit().data());
            break;
        }
        case PropertyModeAuto: {
            status = unicap_set_property_auto(handle, name.toLocal8Bit().data());
            break;
        }
        case PropertyModeOnePush: {
            status = unicap_set_property_one_push(handle, name.toLocal8Bit().data());
            break;
        }
    }

    if (status != STATUS_SUCCESS) {
        qWarning() << "Failed to change property mode!";
    }

    emit propertyChanged();
}


// *********************************************************************
// *                             Capture                               *
// *********************************************************************
void Camera::startCapture ()
{
    if (!captureActive) {
        unicap_status_t status = unicap_start_capture(handle);
        if (status == STATUS_SUCCESS) {
            captureActive = true;
            emit captureStarted();
        } else {
            qWarning() << this << "Failed to start camera capture" << this;
        }
    } else {
        qWarning() << this << "Capture already running!";
    }
}

void Camera::stopCapture ()
{
    if (captureActive) {
        unicap_status_t status = unicap_stop_capture(handle);
        if (status == STATUS_SUCCESS) {
            captureActive = false;
            emit captureFinished();
        } else {
            qWarning() << this << "Failed to stop camera capture" << this;
        }
    }
}

bool Camera::getCaptureState () const
{
    return captureActive;
}


// *********************************************************************
// *                           Frame access                            *
// *********************************************************************
static void callback_new_frame (unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t *buffer, void *usr_data)
{
    reinterpret_cast<Camera *>(usr_data)->captureFrame(buffer);
}

void Camera::captureFrame (unicap_data_buffer_t *buffer)
{
    // Lock
    frameBufferLock.lockForWrite();

    // Resize frame buffer, if necessary
    if (buffer->format.bpp == 8) {
        // Grayscale image
        frameBuffer.create(buffer->format.size.height, buffer->format.size.width, CV_8UC1);
    } else {
        // Color image
        frameBuffer.create(buffer->format.size.height, buffer->format.size.width, CV_8UC3);
    }
    memcpy(frameBuffer.ptr<unsigned char *>(), buffer->data, buffer->buffer_size);

    // Unlock
    frameBufferLock.unlock();

    emit frameReady();
}

void Camera::copyFrame (cv::Mat &frame)
{
    // Copy under lock
    QReadLocker lock(&frameBufferLock);
    frameBuffer.copyTo(frame);
}


} // SourceUnicap
} // Pipeline
} // StereoToolbox
} // MVL
