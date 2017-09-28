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

#ifndef MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__UNICAP__CAMERA_H
#define MVL_STEREO_TOOLBOX__PIPELINE__SOURCES__UNICAP__CAMERA_H

#include <QtWidgets>
#include <unicap.h>

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace Pipeline {
namespace SourceUnicap {


class Camera : public QObject
{
    Q_OBJECT

public:
    Camera (unicap_handle_t handle, QObject *parent = Q_NULLPTR);
    virtual ~Camera ();

    // Config widget
    QWidget *createConfigWidget (QWidget *parent = Q_NULLPTR);

    // Camera identification
    unicap_device_t getDevice () const;
    bool isSameCamera (const unicap_device_t &otherDevice) const;

    // Hardware information
    QString getDeviceVendor () const;
    QString getDeviceModel () const;
    QString getDevicePluginName () const;
    QString getDeviceFileName () const;

    // Format
    const QVector<unicap_format_t> &getSupportedFormats () const;
    void setFormat (const unicap_format_t &newFormat);
    const unicap_format_t &getFormat () const;

    void setSize (unicap_rect_t &size);

    // Camera properties
    const QVector<unicap_property_t> &getSupportedProperties () const;

    void updateProperty (unicap_property_t &property);

    void setPropertyValue (const QString &name, double value);
    void setPropertyValue (const QString &name, const QString &value);

    enum PropertyMode {
        PropertyModeManual,
        PropertyModeAuto,
        PropertyModeOnePush,
    };

    void setPropertyMode (const QString &name, PropertyMode mode);

    // Camera start/stop
    void startCapture ();
    void stopCapture ();

    bool getCaptureState () const;

    // Frame
    void captureFrame (unicap_data_buffer_t *buffer);
    void copyFrame (cv::Mat &frame);

protected:
    void updateFormat();

signals:
    void captureStarted ();
    void captureFinished ();
    void frameReady ();

    void error (QString message);

    void formatChanged ();
    void sizeChanged ();
    void propertyChanged ();

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


} // SourceUnicap
} // Pipeline
} // StereoToolbox
} // MVL


#endif
