/*
 * Unicap Camera: camera
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

#ifndef CAMERA_UNICAP_H
#define CAMERA_UNICAP_H

#include <QtCore>
#include <QtGui>
#include <unicap.h>

#include <opencv2/core/core.hpp>


namespace SourceUnicap {

class Camera : public QObject
{
    Q_OBJECT

public:
    Camera (unicap_handle_t, QObject * = 0);
    virtual ~Camera ();

    // Config widget
    QWidget *createConfigWidget (QWidget * = 0);

    // Camera identification
    unicap_device_t getDevice () const;
    bool isSameCamera (const unicap_device_t &) const;

    // Hardware information
    QString getDeviceVendor () const;
    QString getDeviceModel () const;
    QString getDevicePluginName () const;
    QString getDeviceFileName () const;

    // Format
    const QVector<unicap_format_t> &getSupportedFormats () const;
    void setFormat (const unicap_format_t &);
    const unicap_format_t &getFormat () const;

    void setSize (unicap_rect_t &);

    // Camera properties
    const QVector<unicap_property_t> &getSupportedProperties () const;

    void updateProperty (unicap_property_t &);

    void setPropertyValue (const QString &, double);
    void setPropertyValue (const QString &, const QString &);

    enum PropertyMode {
        PropertyModeManual,
        PropertyModeAuto,
        PropertyModeOnePush,
    };

    void setPropertyMode (const QString &, PropertyMode);

    // Camera start/stop
    void startCapture ();
    void stopCapture ();

    bool getCaptureState () const;

    float getEffectiveCaptureFramerate () const;

    // Frame
    void copyFrame (cv::Mat &);

protected:
    void updateFormat();

signals:
    void captureStarted ();
    void captureFinished ();
    void frameReady ();

    void error (const QString);

    void formatChanged ();
    void sizeChanged ();
    void propertyChanged ();

public:
    void captureFrame (unicap_data_buffer_t *);

protected:
    unicap_handle_t handle;
    unicap_device_t device;

    QVector<unicap_format_t> formats;
    unicap_format_t format;

    QVector<unicap_property_t> properties;

    bool captureActive;

    // Frame
    cv::Mat frameBuffer;
    QReadWriteLock frameBufferLock;
};

}

#endif
